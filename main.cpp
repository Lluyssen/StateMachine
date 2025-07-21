#include "StateMachine.hpp"
#include <iostream>

enum class State {Idle, Running, Jumping, Dead, Reset};

class Logger : public OnTransition<State>, public OnTransitionRefused<State>
{
    public:

        void onTransition(State from, State to) override
        {
            std::cout << "[Transition] " << static_cast<int>(from) << " -> " << static_cast<int>(to) << std::endl;
        }

        void onRefused(State from, State to, const TransitionError& error) override
        {
            std::cout << "[Refused] " << static_cast<int>(from) << " -> " << static_cast<int>(to) << " | Reason: " << static_cast<int>(error) << std::endl;
        }
};

class ExitLogger : public OnExit<State>
{
    public:

        void onExit(State state) override
        {
            std::cout << "[EXIT] Leaving state : " << static_cast<int>(state) << std::endl;
        }
};

class JumpGuard : public OnTransitionGuard<State>
{
    public:

        int stamina = 0;

        bool canTransition(State from, State to) override
        {
            (void)from;
            if (to == State::Jumping && stamina <= 0)
                return false;
            return true;
        }
};

int main(void)
{
    StateMachine<State> sMachine;
    sMachine.setInitialState(State::Idle);
    sMachine.addTransition(StatemachineAny, State::Dead);
    sMachine.addTransition(State::Reset, StatemachineAny);
    sMachine.addTransition(State::Idle, State::Running);
    sMachine.addTransition(State::Running, State::Jumping);

    auto logger = std::make_shared<Logger>();
    auto guard = std::make_shared<JumpGuard>();
    auto exitLogger = std::make_shared<ExitLogger>();

    sMachine.registerOnTransition(logger);
    sMachine.registerOnTransitionRefused(logger);
    sMachine.registerGuard(guard);
    sMachine.registerOnExit(exitLogger);

    sMachine.transitionTo(State::Idle);
    sMachine.transitionTo(State::Running);
    sMachine.transitionTo(State::Jumping);
    guard->stamina = 5;
    sMachine.transitionTo(State::Jumping);
    sMachine.transitionTo(State::Dead);
    sMachine.backToPrevious();

    std::cout << "HISTORY : " << std::endl;
    for (const auto& [from, to] : sMachine.getHistory())
        std::cout << static_cast<int>(from) << " -> " << static_cast<int>(to) << std::endl;

    return 0;
}