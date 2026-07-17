// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

// Scripted fl::IWindow test double for the configure() first-run flow. Folder
// dialog results and message-box button returns are queued by the test; every
// other interface method is inert.

#include "platform/IWindow.h"

#include <deque>
#include <optional>
#include <string>
#include <vector>

namespace fatest {

class FakeWindow : public fl::IWindow {
  public:
    std::deque<std::optional<std::string>> folderResults;
    int folderDialogCount{0};
    std::deque<int> messageBoxReturns; // first button (id of buttons[0]) when empty

    struct MessageBoxCall {
        std::string title;
        std::string message;
        int numButtons;
    };
    std::vector<MessageBoxCall> messageBoxCalls;

    std::optional<std::string> showFolderDialog(const char*, const char*) override {
        ++folderDialogCount;
        if (folderResults.empty())
            return std::nullopt;
        auto result = folderResults.front();
        folderResults.pop_front();
        return result;
    }

    int showMessageBox(MessageBoxType, const char* title, const char* message, const MessageBoxButton* buttons,
                       int numButtons) override {
        messageBoxCalls.push_back({title != nullptr ? title : "", message != nullptr ? message : "", numButtons});
        if (!messageBoxReturns.empty()) {
            const int r = messageBoxReturns.front();
            messageBoxReturns.pop_front();
            return r;
        }
        return (numButtons > 0) ? buttons[0].id : -1;
    }

    // Inert remainder of the interface.
    bool init(const char*, int, int) override {
        return true;
    }
    void shutdown() override {}
    void pollEvents() override {}
    void setEventHandler(fl::IWindowEventHandler*) override {}
    int width() const override {
        return 0;
    }
    int height() const override {
        return 0;
    }
    int logicalWidth() const override {
        return 0;
    }
    int logicalHeight() const override {
        return 0;
    }
    bool shouldClose() const override {
        return false;
    }
    void* nativeHandle() const override {
        return nullptr;
    }
    const char* getLastError() const override {
        return nullptr;
    }
    void openURL(const char*) override {}
    void setTitle(const char*) override {}
    bool setSize(int, int) override {
        return true;
    }
    bool setFullscreen(bool) override {
        return true;
    }
    bool setDisplayMode(const fl::IDisplay::DisplayMode&) override {
        return true;
    }
    int getCurrentMonitorId() const override {
        return 0;
    }
};

} // namespace fatest
