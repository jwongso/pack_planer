#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include "item.h"

/**
 * @brief Represents a pack containing multiple items
 */
class pack {
public:
    /**
     * @brief Construct a new pack object
     * @param pack_number The pack identifier
     */
    explicit pack(int pack_number) noexcept
        : m_pack_number(pack_number) {
        // Reserve some space to avoid initial reallocations
        m_items.reserve(8);
    }

    /**
     * @brief Add item to pack
     * @param item The item to add
     * @param max_items Maximum number of items allowed in the pack
     * @param max_weight Maximum weight allowed in the pack
     * @return bool True if successful, false if constraints violated
     */
    [[nodiscard]] bool add_item(const item& item, int max_items, double max_weight) noexcept {
        int new_quantity = m_total_items + item.get_quantity();
        double new_weight = m_total_weight + item.get_total_weight();

        if (new_quantity <= max_items && new_weight <= max_weight) {
            m_items.push_back(item);
            m_total_items = new_quantity;
            m_total_weight = new_weight;
            m_max_length = std::max(m_max_length, item.get_length());
            return true;
        }
        return false;
    }

    /**
     * @brief Try to add partial quantity of an item (backward compatibility)
     * @param item The item to add
     * @param max_items Maximum number of items allowed in the pack
     * @param max_weight Maximum weight allowed in the pack
     * @return int Number of items successfully added
     */
    [[nodiscard]] int add_partial_item(const item& item, int max_items, double max_weight) noexcept {
        return add_partial_item(item.get_id(), item.get_length(),
                                item.get_quantity(), item.get_weight(), max_items, max_weight);
    }

    /**
     * @brief Try to add partial quantity of an item using individual parameters
     * @param id The item ID
     * @param length The item length
     * @param quantity The item quantity
     * @param weight The item weight per piece
     * @param max_items Maximum number of items allowed in the pack
     * @param max_weight Maximum weight allowed in the pack
     * @return int Number of items successfully added
     */
    [[nodiscard]] int add_partial_item(int id, int length, int quantity, double weight,
                                    int max_items, double max_weight) noexcept {
        // SAFETY: Validate inputs to prevent negative values
        if (quantity <= 0 || max_items <= 0 || max_weight < 0) {
            return 0;
        }

        // SAFETY: Ensure length is positive for valid packing
        length = std::max(1, length);

        // SAFETY: Ensure weight is non-negative
        weight = std::max(0.0, weight);

        const int max_by_items = max_items - m_total_items;
        const double weight_remaining = max_weight - m_total_weight;

        // Handle zero weight case - if weight is 0, weight constraint doesn't apply
        const int max_by_weight = (weight == 0.0) ? quantity :
                                    static_cast<int>(weight_remaining / weight);

        // SAFETY: Ensure max_by_weight is non-negative to prevent underflow
        const int safe_max_by_weight = std::max(0, max_by_weight);

        const int can_add = std::min({max_by_items, safe_max_by_weight, quantity});
        if (can_add > 0) {
            m_items.emplace_back(id, length, can_add, weight);
            m_total_items += can_add;
            m_total_weight += can_add * weight;
            m_max_length = std::max(m_max_length, length);
        }
        return can_add;
    }

    /**
     * @brief Check if the pack is full
     * @param max_items Maximum number of items allowed in the pack
     * @param max_weight Maximum weight allowed in the pack
     * @return bool True if the pack is full
     */
    [[nodiscard]] bool is_full(int max_items, double max_weight) const noexcept {
        // Floating-point precision fix by epsilon.
        return m_total_items >= max_items || m_total_weight >= max_weight - 1e-9;
    }

    /**
     * @brief Get the pack number
     * @return int The pack number
     */
    [[nodiscard]] int get_pack_number() const noexcept { return m_pack_number; }

    /**
     * @brief Set the pack number
     * @param pack_number The pack number
     */
    void set_pack_number(int pack_number) { m_pack_number = pack_number; }

    /**
     * @brief Get the items in the pack
     * @return const std::vector<Item>& Reference to the items vector
     */
    [[nodiscard]] const std::vector<item>& get_items() const noexcept { return m_items; }

    /**
     * @brief Get the total number of items in the pack
     * @return int Total number of items
     */
    [[nodiscard]] int get_total_items() const noexcept { return m_total_items; }

    /**
     * @brief Get the total weight of the pack
     * @return double Total weight
     */
    [[nodiscard]] double get_total_weight() const noexcept { return m_total_weight; }

    /**
     * @brief Get the maximum length of any item in the pack
     * @return int Maximum length
     */
    [[nodiscard]] int get_pack_length() const noexcept { return m_max_length; }

    /**
     * @brief Check if the pack is empty
     * @return bool True if the pack is empty
     */
    [[nodiscard]] bool is_empty() const noexcept { return m_items.empty(); }

    /**
     * @brief Get string representation of the pack
     * @return std::string The string representation
     */
    [[nodiscard]] std::string to_string() const {
        std::ostringstream oss;
        oss << "Pack Number: " << m_pack_number << "\n";

        for (const auto& i : m_items) {
            oss << i.to_string() << "\n";
        }

        oss << "Pack Length: " << get_pack_length()
            << ", Pack Weight: " << std::fixed << std::setprecision(2) << get_total_weight();

        return oss.str();
    }

    /**
     * @brief Get the remaining weight based on the given maximum weight
     * @param max_weight Maximum weight
     * @return double The remaining weight in this Pack
     */
    [[nodiscard]] double get_remaining_weight_capacity(double max_weight) const noexcept {
        return max_weight - get_total_weight();
    }

    /**
     * @brief Get the remaining capacity capacity based on the given maximum capacity
     * @param max_items Maximum capacity
     * @return int The remaining capacity in this Pack
     */
    [[nodiscard]] int get_remaining_item_capacity(int max_items) const noexcept {
        return max_items - get_total_items();
    }

private:
    int m_pack_number = 0;
    std::vector<item> m_items;
    int m_total_items = 0;
    double m_total_weight = 0.0;
    int m_max_length = 0;
};
