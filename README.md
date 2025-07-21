# --- StateMachine<T> – Machine à États Générique en C++ ---

## 🧠 Description
Cette classe template implémente une **machine à états fortement typée** basée sur un type `T` (enum ou struct). Elle prend en charge :

- Transitions explicites (`addTransition(from, to)`)
- Transitions globales et joker (`StatemachineAny`)
- Hiérarchie d'états (`setParentState`)
- Règles de validation (`canTransition`)
- Callbacks : `onEnter`, `onExit`, `onTransition`, `onTransitionRefused`
- Historique complet des transitions
- ⚡ Nouvelle fonctionnalité : `backToPrevious()` pour revenir à l'état précédent

---

## 🧩 Interfaces disponibles

### Transition de base
```cpp
StateMachine<T> machine;
machine.setInitialState(State::Idle);
machine.addTransition(State::Idle, State::Running);
machine.transitionTo(State::Running);
```

### Événements personnalisables (observer pattern)
```cpp
class Logger : public OnTransition<T>, public OnTransitionRefused<T> {
    void onTransition(T from, T to) override;
    void onRefused(T from, T to, TransitionError reason) override;
};
machine.registerOnTransition(std::make_shared<Logger>());
```

### Guards
```cpp
class Guard : public OnTransitionGuard<T> {
    bool canTransition(T from, T to) override {
        return to != T::Dead;
    }
};
```

### Historique et retour
```cpp
machine.getHistory();       // Liste des transitions
machine.backToPrevious();   // Revenir à l’état précédent
```

---

## 🧾 Enum `TransitionError`
| Valeur           | Signification                      |
|------------------|------------------------------------|
| NoInitialState   | Aucune transition possible depuis un état non initialisé |
| AlreadyInTarget  | L’état courant est déjà la cible   |
| InvalidTransition| La transition n’est pas autorisée  |
| BlockedByGuard   | Un guard a empêché la transition   |

---

## 🔍 Utilisation recommandée

| Cas d’usage                    | Pertinence |
|--------------------------------|------------|
| Jeux vidéo / FSM               | ✅✅✅✅      |
| Simulations / IA               | ✅✅✅        |
| Automates métier ou réseau     | ✅✅         |
| Logiciels embarqués            | ✅          |

---

## 💡 Extrait d'utilisation
```cpp
enum class State { Idle, Running, Jumping, Dead };
StateMachine<State> machine;

machine.setInitialState(State::Idle);
machine.addTransition(State::Idle, State::Running);
machine.addTransition(State::Running, State::Jumping);
machine.transitionTo(State::Running);
machine.transitionTo(State::Jumping);
machine.backToPrevious(); // Reviens à Running
```
