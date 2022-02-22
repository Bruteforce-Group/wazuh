/* Copyright (C) 2015-2021, Wazuh Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef _OP_BUILDER_HELPER_FILTER_H
#define _OP_BUILDER_HELPER_FILTER_H

#include "builderTypes.hpp"

/*
 * The helper filter, builds a lifter that will chain rxcpp filter operation
 * Rxcpp filter expects a function that returns bool.
 */

namespace builder::internals::builders
{

/**
 * @brief Create `exists` helper function that filters events that contains specified field.
 *
 * The filter checks if a field exists in the JSON event `e`.
 * For example: if def = `{wazuh: +exists}` only events containing `wazuh` field
 * will continue on the rxcpp pipeline.
 * @param def The filter definition. i.e : `{wazuh: +exists}`
 * @return types::Lifter The lifter with the `exists` filter.
 */
types::Lifter opBuilderHelperExists(const types::DocumentValue & def);

/**
 * @brief Create `not_exists` helper function that filters events that not contains specified field.
 *
 * The filter checks if a field not exists in the JSON event `e`.
 * For example: if def = `{wazuh: +not_exists}` only events not containing `wazuh`
 * field will continue on the rxcpp pipeline.
 * @param def The filter definition. i.e : `{wazuh: +exists}`
 * @return types::Lifter The lifter with the `exists` filter.
 */
types::Lifter opBuilderHelperNotExists(const types::DocumentValue & def);

/**
 * @brief Builds helper integer equal operation.
 * Checks that the field is equal to an integer or another numeric field
 *
 * @param def Definition of the operation to be built
 * @return types::Lifter
 */
types::Lifter opBuilderHelperIntEqual(const types::DocumentValue & def);

/**
 * @brief Compares a string of the event against another string that may or may not belong to the event `e`
 *
 * @param key The key/path of the field to be compared
 * @param op The operator to be used for the comparison. Operators are:
 * - `=`: checks if the field is equal to the value
 * - `!`: checks if the field is not equal to the value
 * - `<`: checks if the field is less than the value
 * - `>`: checks if the field is greater than the value
 * - `m`: checks if the field is less than or equal to the value
 * - `n`: checks if the field is greater than or equal to the value
 * @param e The event containing the field to be compared
 * @param refExpStr The key/path of the field to be compared against (optional)
 * @param expectedStr The string to be compared against (optional)
 * @return true if the comparison is true
 * @return false if the comparison is false
 * @note If `refExpStr` is not provided, the comparison will be against the value of `expectedStr`
 */
inline bool opBuilderHelperStringComparison(const std::string  key, char op, types::Event& e,
                                                 std::optional<std::string> refExpStr,
                                                 std::optional<std::string> expectedStr);

/**
 * @brief Create `s_eq` helper function that filters events with a string
 * field equals to a value.
 *
 * The filter checks if a field in the JSON event `wazuh` is equal to a value.
 * Only pass events if the fields are equal (case sensitive) and the values are a string.
 * @param def The filter definition. i.e : `{wazuh: +s_eq/value}`
 * @return types::Lifter The lifter with the `s_eq` filter.
 * @throw std::runtime_error if the parameter is not a string.
 */
types::Lifter opBuilderHelperString_eq(const types::DocumentValue & def);

/**
 * @brief Create `s_ne` helper function that filters events with a string
 * field not equals to a value.
 *
 * The filter checks if a field in the JSON event `wazuh` is not  equal to a value.
 * Only do not pass events if the fields are equal (case sensitive) and the values are a string.
 * @param def The filter definition. i.e : `{wazuh: +s_ne/value}`
 * @return types::Lifter The lifter with the `s_ne` filter.
 * @throw std::runtime_error if the parameter is not a string.
 */
types::Lifter opBuilderHelperString_ne(const types::DocumentValue & def);

/**
 * @brief Create `s_gt` helper function that filters events with a string
 * field greater than a value.
 *
 * The filter checks if the JSON event field <field> is greater than a <value>
 * or another field <$ref>. Only pass the filter if the event has both fields
 * of type string and passes the condition.
 * use: `<field>: s_gt/<value>|$<ref>`
 * @param def The filter definition. i.e : `{wazuh: +s_gt/value}`
 * @return types::Lifter The lifter with the `s_gt` filter.
 * @throw std::runtime_error if the parameter is not a string.
 */
types::Lifter opBuilderHelperString_gt(const types::DocumentValue & def);


/**
 * @brief Create `s_ge` helper function that filters events with a string
 * field less or equals than a value.
 *
 * The filter checks if the JSON event field <field> is greater or equals than a <value>
 * or another field <$ref>. Only pass the filter if the event has both fields
 * of type string and passes the condition.
 * use: `<field>: s_ge/<value>|$<ref>`
 * @param def The filter definition. i.e : `{wazuh: +s_ge/value}`
 * @return types::Lifter The lifter with the `s_ge` filter.
 * @throw std::runtime_error if the parameter is not a string.
 */
types::Lifter opBuilderHelperString_ge(const types::DocumentValue & def);


/**
 * @brief Create `s_lt` helper function that filters events with a string
 * field less than a value.
 *
 * The filter checks if the JSON event field <field> is less than a <value>
 * or another field <$ref>. Only pass the filter if the event has both fields
 * of type string and passes the condition.
 * use: `<field>: s_lt/<value>|$<ref>`
 * @param def The filter definition. i.e : `{wazuh: +s_lt/value}`
 * @return types::Lifter The lifter with the `s_lt` filter.
 * @throw std::runtime_error if the parameter is not a string.
 */
types::Lifter opBuilderHelperString_lt(const types::DocumentValue & def);


/**
 * @brief Create `s_le` helper function that filters events with a string
 * field less or equals than a value.
 *
 * The filter checks if the JSON event field <field> is less or equals than a <value>
 * or another field <$ref>. Only pass the filter if the event has both fields
 * of type string and passes the condition.
 * use: `<field>: s_le/<value>|$<ref>`
 * @param def The filter definition. i.e : `{wazuh: +s_le/value}`
 * @return types::Lifter The lifter with the `s_le` filter.
 * @throw std::runtime_error if the parameter is not a string.
 */
types::Lifter opBuilderHelperString_le(const types::DocumentValue & def);


} // namespace builder::internals::builders

#endif // _OP_BUILDER_HELPER_FILTER_H
