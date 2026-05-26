#include "ConfigLoader.h"
#include "Json.h"
#include "Actions.h"
#include "Types.h"
#include <Windows.h>
#include <fstream>
#include <vector>


WORD ConfigLoader::keyNameToVK(const std::string& name) {
    if (name.size() == 1 && name[0] >= 'A' && name[0] <= 'Z')
        return static_cast<WORD>(name[0]);
    if (name.size() == 1 && name[0] >= '0' && name[0] <= '9')
        return static_cast<WORD>(name[0]);

    static const std::unordered_map<std::string, int> table = {
        { "CTRL",        VK_CONTROL          }, { "SHIFT",       VK_SHIFT            },
        { "ALT",         VK_MENU             }, { "WIN",         VK_LWIN             },
        { "LCTRL",       VK_LCONTROL         }, { "RCTRL",       VK_RCONTROL         },
        { "LSHIFT",      VK_LSHIFT           }, { "RSHIFT",      VK_RSHIFT           },
        { "LALT",        VK_LMENU            }, { "RALT",        VK_RMENU            },
        { "SPACE",       VK_SPACE            }, { "ENTER",       VK_RETURN           },
        { "ESCAPE",      VK_ESCAPE           }, { "TAB",         VK_TAB              },
        { "BACKSPACE",   VK_BACK             }, { "DELETE",      VK_DELETE           },
        { "INSERT",      VK_INSERT           }, { "HOME",        VK_HOME             },
        { "END",         VK_END              }, { "PGUP",        VK_PRIOR            },
        { "PGDN",        VK_NEXT             }, { "LEFT",        VK_LEFT             },
        { "RIGHT",       VK_RIGHT            }, { "UP",          VK_UP               },
        { "DOWN",        VK_DOWN             }, { "F1",          VK_F1               },
        { "F2",          VK_F2               }, { "F3",          VK_F3               },
        { "F4",          VK_F4               }, { "F5",          VK_F5               },
        { "F6",          VK_F6               }, { "F7",          VK_F7               },
        { "F8",          VK_F8               }, { "F9",          VK_F9               },
        { "F10",         VK_F10              }, { "F11",         VK_F11              },
        { "F12",         VK_F12              }, { "NUM0",        VK_NUMPAD0          },
        { "NUM1",        VK_NUMPAD1          }, { "NUM2",        VK_NUMPAD2          },
        { "NUM3",        VK_NUMPAD3          }, { "NUM4",        VK_NUMPAD4          },
        { "NUM5",        VK_NUMPAD5          }, { "NUM6",        VK_NUMPAD6          },
        { "NUM7",        VK_NUMPAD7          }, { "NUM8",        VK_NUMPAD8          },
        { "NUM9",        VK_NUMPAD9          }, { "NUM_MUL",     VK_MULTIPLY         },
        { "NUM_ADD",     VK_ADD              }, { "NUM_SUB",     VK_SUBTRACT         },
        { "NUM_DOT",     VK_DECIMAL          }, { "NUM_DIV",     VK_DIVIDE           },
        { "OEM_PLUS",    VK_OEM_PLUS         }, { "OEM_MINUS",   VK_OEM_MINUS        },
        { "OEM_COMMA",   VK_OEM_COMMA        }, { "OEM_PERIOD",  VK_OEM_PERIOD       },
        { "OEM_1",       VK_OEM_1            }, { "OEM_2",       VK_OEM_2            },
        { "OEM_3",       VK_OEM_3            }, { "OEM_4",       VK_OEM_4            },
        { "OEM_5",       VK_OEM_5            }, { "OEM_6",       VK_OEM_6            },
        { "OEM_7",       VK_OEM_7            }, { "VOL_UP",      VK_VOLUME_UP        },
        { "VOL_DOWN",    VK_VOLUME_DOWN      }, { "VOL_MUTE",    VK_VOLUME_MUTE      },
        { "MEDIA_PLAY",  VK_MEDIA_PLAY_PAUSE }, { "MEDIA_NEXT",  VK_MEDIA_NEXT_TRACK },
        { "MEDIA_PREV",  VK_MEDIA_PREV_TRACK }, { "PRINTSCREEN", VK_SNAPSHOT         },
        { "SCROLLLOCK",  VK_SCROLL           }, { "PAUSE",       VK_PAUSE            },
        { "CAPSLOCK",    VK_CAPITAL          },
    };

    auto it = table.find(name);
    return (it != table.end()) ? static_cast<WORD>(it->second) : 0;
}


void ConfigLoader::writeDefaultConfig(const std::string& path) {
    static const char* DEFAULT_JSON = R"({
  "device": "Launchpad",
  "pages": [
    { "name": "HOME",      "pads": [] },
    { "name": "LIGHTSHOW", "pads": [] }
  ]
})";
    std::ofstream f(path);
    if (f.is_open()) f << DEFAULT_JSON;
}


bool ConfigLoader::load(const std::string& path, PageManager& pm) {
    {
        std::ifstream probe(path);
        if (!probe.is_open()) writeDefaultConfig(path);
    }

    Json root;
    try {
        root = Json::parseFile(path);
    } catch (const std::exception&) {
        return false;
    }

    if (!root.isObject() || !root.has("pages")) return false;

    for (const Json& pageJson : root["pages"].toArray()) {
        const std::string pageName = pageJson.getString("name", "UNNAMED");
        PadMap padMap;

        for (const Json& padJson : pageJson["pads"].toArray()) {
            if (!padJson.has("note")) continue;

            const uint8_t note  = static_cast<uint8_t>(padJson.getInt("note", 0));
            const uint8_t color = LP2::colorFromName(padJson.getString("color", "OFF"));

            PadConfig cfg;
            cfg.color = color;

            if (padJson.has("action")) {
                const Json&       act  = padJson["action"];
                const std::string type = act.getString("type", "none");

                auto parseKeys = [&]() {
                    std::vector<WORD> vkeys;
                    for (const Json& k : act["keys"].toArray()) {
                        WORD vk = keyNameToVK(k.getString());
                        if (vk != 0) vkeys.push_back(vk);
                    }
                    return vkeys;
                };

                if (type == "keys") {
                    cfg.onPress = Actions::sendKeysVec(parseKeys());

                } else if (type == "launch") {
                    cfg.onPress = Actions::launchApp(
                        act.getString("path", ""),
                        act.getString("args", ""));

                } else if (type == "toggle") {
                    cfg.isToggle = true;
                    cfg.colorOn  = LP2::colorFromName(act.getString("colorOn",  "GREEN"));
                    cfg.colorOff = LP2::colorFromName(act.getString("colorOff", "RED"));
                    cfg.color    = cfg.colorOff;
                    auto vkeys   = parseKeys();
                    if (!vkeys.empty())
                        cfg.onPress = Actions::sendKeysVec(std::move(vkeys));
                }
            }

            padMap[note] = std::move(cfg);
        }

        pm.addPage(pageName, std::move(padMap));
    }

    return true;
}
