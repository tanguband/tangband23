#pragma once

#include <memory>
#include <variant>

struct no_class_specific_data {
};
struct smith_data_type;
struct force_trainer_data_type;
struct bluemage_data_type;
struct magic_eater_data_type;
struct bard_data_type;
struct mane_data_type;
class SniperData;
struct samurai_data_type;
struct monk_data_type;
struct ninja_data_type;
struct spell_hex_data_type;

using ClassSpecificData = std::variant<

    no_class_specific_data,
    std::shared_ptr<smith_data_type>,
    std::shared_ptr<force_trainer_data_type>,
    std::shared_ptr<bluemage_data_type>,
    std::shared_ptr<magic_eater_data_type>,
    std::shared_ptr<bard_data_type>,
    std::shared_ptr<mane_data_type>,
    std::shared_ptr<SniperData>,
    std::shared_ptr<samurai_data_type>,
    std::shared_ptr<monk_data_type>,
    std::shared_ptr<ninja_data_type>,
    std::shared_ptr<spell_hex_data_type>

    >;
