#include <Geode/Geode.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>

using namespace geode::prelude;

static int g_targetFPS = 240;
static bool g_enabled = true;
static bool g_disableVSync = false;

class $modify(FPSDirector, CCDirector) {
    void setAnimationInterval(double interval) {
        if (!g_enabled) {
            CCDirector::setAnimationInterval(interval);
            return;
        }
        int effectiveFPS = g_disableVSync ? 1000 : g_targetFPS;
        CCDirector::setAnimationInterval(1.0 / static_cast<double>(effectiveFPS));
    }
};

class $modify(FPSPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        auto menu = this->getChildByID("center-button-menu");
        if (!menu) return;
        auto fpsBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("FPS", 40, true, "goldFont.fnt", "GJ_button_01.png", 30, 1),
            this,
            menu_selector(FPSPauseLayer::onFPSButton)
        );
        fpsBtn->setID("fps-bypass-btn");
        menu->addChild(fpsBtn);
        menu->updateLayout();
    }
    
    void onFPSButton(CCObject*) {
        auto input = TextInput::create(120, "FPS", "chatFont.fnt");
        input->setString(std::to_string(g_targetFPS));
        input->setCallback([](std::string const& str) {
            try {
                int fps = std::stoi(str);
                if (fps >= 30 && fps <= 1000) {
                    g_targetFPS = fps;
                    Mod::get()->setSettingValue("fps-value", fps);
                    auto dir = CCDirector::sharedDirector();
                    if (dir) dir->setAnimationInterval(1.0 / 60.0);
                }
            } catch (...) {}
        });
        FLAlertLayer::create(nullptr, "FPS Bypass", "OK", nullptr, 300, input)->show();
    }
};

$on_mod(Loaded) {
    auto mod = Mod::get();
    g_enabled = mod->getSettingValue<bool>("enabled");
    g_targetFPS = mod->getSettingValue<int>("fps-value");
    g_disableVSync = mod->getSettingValue<bool>("disable-vsync");
    
    if (g_enabled) {
        auto dir = CCDirector::sharedDirector();
        if (dir) {
            int fps = g_disableVSync ? 1000 : g_targetFPS;
            dir->setAnimationInterval(1.0 / static_cast<double>(fps));
        }
    }
    
    listenForSettingChanges("enabled", [](bool value) {
        g_enabled = value;
        auto dir = CCDirector::sharedDirector();
        if (!dir) return;
        dir->setAnimationInterval(value ? 1.0 / static_cast<double>(g_disableVSync ? 1000 : g_targetFPS) : 1.0 / 60.0);
    });
    
    listenForSettingChanges("fps-value", [](int value) {
        g_targetFPS = value;
        if (g_enabled && !g_disableVSync) {
            auto dir = CCDirector::sharedDirector();
            if (dir) dir->setAnimationInterval(1.0 / static_cast<double>(value));
        }
    });
    
    listenForSettingChanges("disable-vsync", [](bool value) {
        g_disableVSync = value;
        if (g_enabled) {
            auto dir = CCDirector::sharedDirector();
            if (dir) {
                int fps = value ? 1000 : g_targetFPS;
                dir->setAnimationInterval(1.0 / static_cast<double>(fps));
            }
        }
    });
}
