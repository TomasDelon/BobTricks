#pragma once

namespace bobtricks {

/**
 * \brief Commande de debug minimale pour les evolutions futures.
 */
enum class DebugCommandType {
    None
};

/**
 * \brief Valeur transportee par une commande de debug.
 */
struct DebugCommand {
    DebugCommandType type {DebugCommandType::None};
};

}  // namespace bobtricks
