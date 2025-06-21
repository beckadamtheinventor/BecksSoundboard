#pragma once

#include <vector>
namespace Menus {
    class Menu {
        public:
        virtual void show() {}
    };

    class MenuManager : std::vector<Menu*> {
        public:
        void show() {
            for (int i=0; i<size(); i++) {
                if (at(i)) at(i)->show();
            }
        }
        template<class T>
        T* add(T* menu) {
            push_back(menu);
            return at(size() - 1);
        }
    };

    class OptionsMenu : public Menu {
        public:
        void show() override;
    };

    class SoundsMenu : public Menu {
        public:
        void show() override;
    };

    class ConsoleMenu : public Menu {
        public:
        void show() override;
    };



}