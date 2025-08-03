#include <concepts>
#include <memory>
#include <string>
#include <vector>

namespace clanguml {
namespace t00101 {

// Strategy pattern implementation using static polymorphism with C++20 concepts

// Example context: Text processing with different formatting strategies

// C++20 concept for Strategy interface using static polymorphism
template<typename T>
concept FormattingStrategyConcept = requires(T t, const std::string &text) {
    { t.format(text) } -> std::same_as<std::string>;
    { t.get_name() } -> std::same_as<std::string>;
};

// Template base for concrete strategies (CRTP)
template<typename Derived>
class FormattingStrategyBase {
public:
    std::string format(const std::string &text) {
        return static_cast<Derived*>(this)->format_impl(text);
    }

    std::string get_name() {
        return static_cast<Derived*>(this)->get_name_impl();
    }

protected:
    FormattingStrategyBase() = default;
};

// Concrete Strategy for plain text formatting
class PlainTextStrategy : public FormattingStrategyBase<PlainTextStrategy> {
public:
    std::string format_impl(const std::string &text) {
        return text;
    }

    std::string get_name_impl() {
        return "PlainText";
    }
};


// Concrete Strategy for markdown formatting
class MarkdownStrategy : public FormattingStrategyBase<MarkdownStrategy> {
public:
    std::string format_impl(const std::string &text) {
        return "# " + text + "\n";
    }

    std::string get_name_impl() {
        return "Markdown";
    }
};


// Context template using static polymorphism and C++20 concepts
template<FormattingStrategyConcept StrategyType>
class TextProcessor {
public:
    explicit TextProcessor(StrategyType &strategy) : strategy_(strategy) {}

    std::string process_text(const std::string &text) {
        return strategy_.format(text);
    }

    std::string get_strategy_name() {
        return strategy_.get_name();
    }

    void set_strategy(StrategyType &new_strategy) {
        strategy_ = new_strategy;
    }

private:
    StrategyType &strategy_;
};

// Client code demonstrating the Strategy pattern with static polymorphism
class DocumentProcessor {
public:
    std::string process_as_plain_text(const std::string &content) {
        PlainTextStrategy strategy;
        TextProcessor processor(strategy);
        return processor.process_text(content);
    }


    std::string process_as_markdown(const std::string &content) {
        MarkdownStrategy strategy;
        TextProcessor processor(strategy);
        return processor.process_text(content);
    }


    template<FormattingStrategyConcept StrategyType>
    std::string process_with_custom_strategy(const std::string &content,
                                           StrategyType &strategy) {
        TextProcessor processor(strategy);
        return processor.process_text(content);
    }
};

// Factory for creating different strategies
class StrategyFactory {
public:
    enum class StrategyType {
        PlainText,
        Markdown
    };

    static std::unique_ptr<PlainTextStrategy> create_plain_text_strategy() {
        return std::make_unique<PlainTextStrategy>();
    }


    static std::unique_ptr<MarkdownStrategy> create_markdown_strategy() {
        return std::make_unique<MarkdownStrategy>();
    }

};

} // namespace t00101
} // namespace clanguml