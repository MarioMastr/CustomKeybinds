#include <Geode/modify/UILayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include "../include/Keybinds.hpp"

using namespace geode::prelude;
using namespace keybinds;

static void addBindSprites(CCNode* target, const char* action) {
    target->removeAllChildren();

    auto bindContainer = CCNode::create();
    bindContainer->setScale(.65f);
    bool first = true;
    for (auto& bind : BindManager::get()->getBindsFor(action)) {
        if (!first) {
            bindContainer->addChild(CCLabelBMFont::create("/", "bigFont.fnt"));
        }
        first = false;
        bindContainer->addChild(bind->createLabel());
    }
    bindContainer->setID("binds"_spr);
    bindContainer->setContentSize({
        target->getContentSize().width / bindContainer->getScale(), 40.f
    });
    bindContainer->setLayout(RowLayout::create());
    bindContainer->setAnchorPoint({ .5f, .5f });
    bindContainer->setPosition(target->getContentSize().width / 2, -1.f);
    target->addChild(bindContainer);
}

struct $modify(PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        this->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {
            if (event->isDown()) {
                this->onResume(nullptr);
                return ListenerResult::Stop;
            }
            return ListenerResult::Propagate;
        }, "robtop.geometry-dash/unpause-level");

        this->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {
            if (event->isDown()) {
                this->onQuit(nullptr);
                return ListenerResult::Stop;
            }
            return ListenerResult::Propagate;
        }, "robtop.geometry-dash/exit-level");

        this->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {
            if (event->isDown()) {
                if(PlayLayer::get() && PlayLayer::get()->m_isPracticeMode) {
                    this->onNormalMode(nullptr);
                } else {
                    this->onPracticeMode(nullptr);
                }
                return ListenerResult::Stop;
            }
            return ListenerResult::Propagate;
        }, "robtop.geometry-dash/practice-level");

        this->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {
            if (event->isDown()) {
                this->onRestart(nullptr);
                return ListenerResult::Stop;
            }
            return ListenerResult::Propagate;
        }, "robtop.geometry-dash/restart-level");
    }

    void keyDown(enumKeyCodes key) {
        if (key == enumKeyCodes::KEY_Escape) {
            PauseLayer::keyDown(key);
        }
    }
};

struct $modify(UILayer) {
    static void onModify(auto& self) {
        (void)self.setHookPriority("UILayer::keyDown", 1000);
        (void)self.setHookPriority("UILayer::keyUp", 1000);
    }

    static inline int platformButton() {
        #ifdef GEODE_IS_MACOS
            return 1;
        #else
            return 0;
        #endif
    }

    bool init() {
        if (!UILayer::init())
            return false;
        
        this->defineKeybind("robtop.geometry-dash/jump-p1", [=](bool down) {
            // todo: event priority
            if (PlayLayer::get() && CCDirector::get()->getSmoothFixCheck()) {
                if (down) {
                    PlayLayer::get()->pushButton(platformButton(), true);
                }
                else {
                    PlayLayer::get()->releaseButton(platformButton(), true);
                }
            }
        });
        this->defineKeybind("robtop.geometry-dash/jump-p2", [=](bool down) {
            if (PlayLayer::get() && CCDirector::get()->getSmoothFixCheck()) {
                if (down) {
                    PlayLayer::get()->pushButton(platformButton(), false);
                }
                else {
                    PlayLayer::get()->releaseButton(platformButton(), false);
                }
            }
        });
        this->defineKeybind("robtop.geometry-dash/place-checkpoint", [=](bool down) {
            if (down && PlayLayer::get() && PlayLayer::get()->m_isPracticeMode) {
                this->onCheck(nullptr);
            }
        });
        this->defineKeybind("robtop.geometry-dash/delete-checkpoint", [=](bool down) {
            if (down && PlayLayer::get() && PlayLayer::get()->m_isPracticeMode) {
                this->onDeleteCheck(nullptr);
            }
        });
        this->defineKeybind("robtop.geometry-dash/pause-level", [=](bool down) {
            if (down && PlayLayer::get() && CCDirector::get()->getSmoothFixCheck()) {
                PlayLayer::get()->pauseGame(true);
            }
        });

        // display practice mode button keybinds
        if (auto menu = this->getChildByID("checkpoint-menu")) {
            if (auto add = menu->getChildByID("add-checkpoint-button")) {
                addBindSprites(
                    static_cast<CCMenuItemSpriteExtra*>(add)->getNormalImage(),
                    "robtop.geometry-dash/place-checkpoint"
                );
            }
            if (auto rem = menu->getChildByID("remove-checkpoint-button")) {
                addBindSprites(
                    static_cast<CCMenuItemSpriteExtra*>(rem)->getNormalImage(),
                    "robtop.geometry-dash/delete-checkpoint"
                );
            }
        }
        
        return true;
    }

    void defineKeybind(const char* id, std::function<void(bool)> callback) {
        // adding the events to playlayer instead
        PlayLayer::get()->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {
            callback(event->isDown());
            return ListenerResult::Propagate;
        }, id);
    }

    void keyDown(enumKeyCodes key) {
        if (key == enumKeyCodes::KEY_Escape) {
            UILayer::keyDown(key);
        }
    }
    void keyUp(enumKeyCodes) {}
}
