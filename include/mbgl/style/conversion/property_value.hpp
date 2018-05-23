#pragma once

#include <mbgl/style/property_value.hpp>
#include <mbgl/style/conversion.hpp>
#include <mbgl/style/conversion/constant.hpp>
#include <mbgl/style/conversion/function.hpp>
#include <mbgl/style/expression/value.hpp>
#include <mbgl/style/expression/is_constant.hpp>
#include <mbgl/style/expression/is_expression.hpp>
#include <mbgl/style/expression/find_zoom_curve.hpp>
#include <mbgl/style/expression/parsing_context.hpp>

namespace mbgl {
namespace style {
namespace conversion {

template <class T>
struct Converter<PropertyValue<T>> {
    optional<PropertyValue<T>> operator()(const Convertible& value, Error& error) const {
        using namespace mbgl::style::expression;

        if (isUndefined(value)) {
            return PropertyValue<T>();
        } else if (isExpression(value)) {
            ParsingContext ctx(valueTypeToExpressionType<T>());
            ParseResult expression = ctx.parseLayerPropertyExpression(value);
            if (!expression) {
                error = { ctx.getCombinedErrors() };
                return {};
            }

            if (!isFeatureConstant(**expression)) {
                error = { "property expressions not supported" };
                return {};
            } else if (!isZoomConstant(**expression)) {
                return { CameraFunction<T>(std::move(*expression)) };
            } else {
                auto literal = dynamic_cast<Literal*>(expression->get());
                assert(literal);
                optional<T> constant = fromExpressionValue<T>(literal->getValue());
                if (!constant) {
                    return {};
                }
                return PropertyValue<T>(*constant);
            }
        } else if (isObject(value)) {
            optional<CameraFunction<T>> function = convert<CameraFunction<T>>(value, error);
            if (!function) {
                return {};
            }
            return { *function };
        } else {
            optional<T> constant = convert<T>(value, error);
            if (!constant) {
                return {};
            }
            return { *constant };
        }
    }
};

} // namespace conversion
} // namespace style
} // namespace mbgl
