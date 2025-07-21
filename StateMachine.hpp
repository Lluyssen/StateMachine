#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <iostream>
#include <string>
#include <optional>

struct StateMachineAny_t {};

static constexpr StateMachineAny_t StatemachineAny{};

enum class TransitionError
{
    NoInitialState,
    AlreadyInTarget,
    InvalidTransition,
    BlockedByGuard
};

template <typename T>
class OnEnter
{
    public:

        virtual ~OnEnter() = default;
        virtual void onEnter(T) = 0;
};

template <typename T>
class OnExit
{
    public:

        virtual ~OnExit() = default;
        virtual void onExit(T) = 0;
};

template <typename T>
class OnTransition
{
    public:

        virtual ~OnTransition() = default;
        virtual void onTransition(T, T) = 0;
};

template <typename T>
class OnTransitionRefused
{
    public:

        virtual ~OnTransitionRefused() = default;
        virtual void onRefused(T, T, const TransitionError&) = 0;
};

template <typename T>
class OnTransitionGuard
{
    public:

        virtual ~OnTransitionGuard() = default;
        virtual bool canTransition(T, T) = 0;
};

template <typename T>
class StateMachine
{
    private :

        T _current;
        bool _hasState = false;

        std::unordered_map<T, std::unordered_set<T>> _transition;
        std::unordered_set<T> _globalTransitions;
        std::unordered_set<T> _wildCardSources;
        std::vector<std::pair<T, T>> _history;
        std::unordered_map<T, T> _hierarchy;
        std::vector<std::shared_ptr<OnEnter<T>>> _onEnter;
        std::vector<std::shared_ptr<OnExit<T>>> _onExit;
        std::vector<std::shared_ptr<OnTransition<T>>> _onTransition;
        std::vector<std::shared_ptr<OnTransitionRefused<T>>> _onTransitionRefused;
        std::vector<std::shared_ptr<OnTransitionGuard<T>>> _onTransitionGuard;

        void notifyRefused(T from, T to, const TransitionError& reason) const
        {
            for (const auto& listener : _onTransitionRefused)
                listener->onRefused(from, to, reason);
        }

        bool isValidTransition(T from, T to) const
        {
            auto it = _transition.find(from);
            if (it != _transition.end() && it->second.count(to) > 0)
                return true;
            if (_globalTransitions.count(to) > 0)
                return true;
            if (_wildCardSources.count(from) > 0)
                return true;
            return false;
        }

    public:

        void setInitialState(T state)
        {
            _current = state;
            _hasState = true;
        }

        void addTransition(T from, T to)
        {
            _transition[from].insert(to);
        }

        void addTransition(T from, StateMachineAny_t)
        {
            _wildCardSources.insert(from);
        }

        void addTransition(StateMachineAny_t, T to)
        {
            _globalTransitions.insert(to);
        }

        void setParentState(T child, T parent)
        {
            _hierarchy[child] = parent;
        }

        bool isSubstateOf(T child, T parent) const
        {
            T current = child;
            while (_hierarchy.find(current) != _hierarchy.end())
            {
                current = _hierarchy.at(current);
                if (current == parent)
                    return true;
            }
            return false;
        }

        bool transitionTo(T target)
        {
            if (!_hasState)
            {
                notifyRefused(_current, target, TransitionError::NoInitialState);
                return false;
            }
            if (_current == target)
            {
                notifyRefused(_current, target, TransitionError::AlreadyInTarget);
                return false;
            }
            if (!isValidTransition(_current, target))
            {
                notifyRefused(_current, target, TransitionError::InvalidTransition);
                return false;
            }
            for (const auto& guard : _onTransitionGuard)
            {
                if (!guard->canTransition(_current, target))
                {
                    notifyRefused(_current, target, TransitionError::BlockedByGuard);
                    return false;
                }
            }
            for (auto& listener : _onExit)
                listener->onExit(_current);
            for (auto& listener : _onTransition)
                listener->onTransition(_current, target);
            _history.emplace_back(_current, target);
            _current = target;
            for (auto& listener : _onEnter)
                listener->onEnter(_current);
            return true;
        }

        void registerOnEnter(std::shared_ptr<OnEnter<T>> l)
        {
            _onEnter.push_back(l);
        }

        void registerOnExit(std::shared_ptr<OnExit<T>> l)
        {
            _onExit.push_back(l);
        }

        void registerOnTransition(std::shared_ptr<OnTransition<T>> l)
        {
            _onTransition.push_back(l);
        }

        void registerOnTransitionRefused(std::shared_ptr<OnTransitionRefused<T>> l)
        {
            _onTransitionRefused.push_back(l);
        }

        void registerGuard(std::shared_ptr<OnTransitionGuard<T>> l)
        {
            _onTransitionGuard.push_back(l);
        }

        T getCurrentState(void) const
        {
            return _current;
        }

        const std::vector<std::pair<T, T>>& getHistory(void) const
        {
            return _history;
        }

        bool backToPrevious(void)
        {
            if (_history.empty())
                return false;
            return transitionTo(_history.end()->first);
        }
};