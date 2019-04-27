#ifndef EVENTS_H
#define EVENTS_H

#include "engine.h"
#include <map>

struct EntityDestroyedEvent {
    ECS::Entity entity;
};

struct EventBaseQueue {
    virtual void emit() = 0;
};

template<typename E>
struct EventQueue : public EventBaseQueue {
    private:
    std::vector<E> _queue;
    std::vector<std::function<void(E)>> _listeners;

    public:
    void push(E e) {
        _queue.push_back(e);
    }

    void listen(std::function<void(E)> f) {
        _listeners.push_back(f);
    }

    void emit() override {
        for(auto e : _queue) {
            for(auto l : _listeners) {
                l(e);
            }
        }
        _queue.clear();
    }
};

struct EventHub {
    private:
        std::map<size_t, EventBaseQueue*> const _queues {
            { TypeID::value<EntityDestroyedEvent>(), new EventQueue<EntityDestroyedEvent>() }
        };

        class TypeID {
            static size_t counter;

        public:
            template<typename T>
            static size_t value() {
                static size_t id = counter++;
                return id;
            }
        };
        
    public:
        template<typename T>
        void push(T e) {
            auto q = static_cast<EventQueue<T>*>(_queues.at(TypeID::value<T>()));
            q->push(e);
        }

        template<typename T>
        void listen(std::function<void(T)> f) {
            auto q = static_cast<EventQueue<T>*>(_queues.at(TypeID::value<T>()));
            q->listen(f);
        }

        void emit() {
            for(auto &q : _queues) {
                q.second->emit();
            }
        }
};

#endif