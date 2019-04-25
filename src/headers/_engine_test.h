#ifndef _ENGINE_TEST_H
#define _ENGINE_TEST_H

#include "engine.h"

struct TestComp1 {
    int a = 4;
};

struct TestComp2 {
    int b = 4;
};

void ecs_test1(ECS::ArchetypeManager &arch_manager);

void engine_test() {
    ECS::ArchetypeManager arch_manager;
    
    ecs_test1(arch_manager);
}

void ecs_test1(ECS::ArchetypeManager &arch_manager) {
    auto arch = arch_manager.create_archetype<TestComp1>(2);
    
    auto e = arch_manager.create_entity(arch);
    arch_manager.set_component(e, TestComp1 { 42 });

    Engine::logn("\t has position: %d", arch_manager.has_component<Position>(e) ? 1 : 0);
    Engine::logn("\t has TestComp1: %d", arch_manager.has_component<TestComp1>(e) ? 1 : 0);
    
    Engine::logn("\t Test (should write 42):");
    auto ci2 = arch_manager.get_iterator<TestComp1>();
	for(auto c : ci2.containers) {
        for(int i = 0; i < c->length; i++) {
			auto &t1 = c->index<TestComp1>(i);
            Engine::logn("%d", t1.a);
        }
    }

    Engine::logn("\t Test (should be nothing):");
    ci2 = arch_manager.get_iterator<TestComp1, TestComp2>();
	for(auto c : ci2.containers) {
        for(int i = 0; i < c->length; i++) {
			auto &t1 = c->index<TestComp1>(i);
            Engine::logn("%d", t1.a);

            auto &t2 = c->index<TestComp2>(i);
            Engine::logn("%d", t2.b);
        }
    }
    
    arch_manager.add_component(e, TestComp2 { 66 });

    Engine::logn("\t has position: %d", arch_manager.has_component<Position>(e) ? 1 : 0);
    Engine::logn("\t has TestComp1: %d", arch_manager.has_component<TestComp1>(e) ? 1 : 0);
    Engine::logn("\t has TestComp2: %d", arch_manager.has_component<TestComp2>(e) ? 1 : 0);

    Engine::logn("\t Test (should write 42):");
    ci2 = arch_manager.get_iterator<TestComp1>();
	for(auto c : ci2.containers) {
        for(int i = 0; i < c->length; i++) {
			auto &t1 = c->index<TestComp1>(i);
            Engine::logn("%d", t1.a);
        }
    }

    Engine::logn("\t Test (should write 42 and 66):");
    ci2 = arch_manager.get_iterator<TestComp1, TestComp2>();
	for(auto c : ci2.containers) {
        for(int i = 0; i < c->length; i++) {
			auto &t1 = c->index<TestComp1>(i);
            Engine::logn("%d", t1.a);

            auto &t2 = c->index<TestComp2>(i);
            Engine::logn("%d", t2.b);
        }
    }

    arch_manager.remove_entity(e);

    Engine::logn("\t Test (should write nothing)");
    ci2 = arch_manager.get_iterator<TestComp1>();
	for(auto c : ci2.containers) {
        for(int i = 0; i < c->length; i++) {
			auto &t1 = c->index<TestComp1>(i);
            Engine::logn("%d", t1.a);
        }
    }

    Engine::logn("\t Test (should write nothing)");
    ci2 = arch_manager.get_iterator<TestComp1, TestComp2>();
	for(auto c : ci2.containers) {
        for(int i = 0; i < c->length; i++) {
			auto &t1 = c->index<TestComp1>(i);
            Engine::logn("%d", t1.a);

            auto &t2 = c->index<TestComp2>(i);
            Engine::logn("%d", t2.b);
        }
    }
}

#endif