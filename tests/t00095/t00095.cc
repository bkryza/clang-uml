#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace clanguml {
namespace t00095 {

// Prototype pattern implementation: Document cloning system

// Abstract Prototype interface
class Document {
public:
    virtual ~Document() = default;

    // The clone method is the core of the Prototype pattern
    virtual std::unique_ptr<Document> clone() const = 0;

    virtual void set_title(const std::string &title) = 0;
    virtual void set_content(const std::string &content) = 0;
    virtual std::string get_title() const = 0;
    virtual std::string get_content() const = 0;
    virtual std::string get_type() const = 0;
    virtual void display() const = 0;

protected:
    std::string title_;
    std::string content_;
};

// Concrete Prototype: Text Document
class TextDocument : public Document {
public:
    TextDocument() = default;

    TextDocument(const std::string &title, const std::string &content)
        : font_family_("Arial")
        , font_size_(12)
    {
        title_ = title;
        content_ = content;
    }

    // Copy constructor for cloning
    TextDocument(const TextDocument &other)
        : font_family_(other.font_family_)
        , font_size_(other.font_size_)
    {
        title_ = other.title_;
        content_ = other.content_;
    }

    std::unique_ptr<Document> clone() const override
    {
        return std::make_unique<TextDocument>(*this);
    }

    void set_title(const std::string &title) override { title_ = title; }
    void set_content(const std::string &content) override
    {
        content_ = content;
    }
    std::string get_title() const override { return title_; }
    std::string get_content() const override { return content_; }
    std::string get_type() const override { return "TextDocument"; }
    void display() const override { }

    void set_font_family(const std::string &font) { font_family_ = font; }
    void set_font_size(int size) { font_size_ = size; }
    std::string get_font_family() const { return font_family_; }
    int get_font_size() const { return font_size_; }

private:
    std::string font_family_;
    int font_size_;
};

// Concrete Prototype: Spreadsheet Document
class SpreadsheetDocument : public Document {
public:
    SpreadsheetDocument() = default;

    SpreadsheetDocument(const std::string &title, const std::string &content)
        : rows_(10)
        , columns_(10)
    {
        title_ = title;
        content_ = content;
    }

    // Copy constructor for cloning
    SpreadsheetDocument(const SpreadsheetDocument &other)
        : rows_(other.rows_)
        , columns_(other.columns_)
        , formulas_(other.formulas_)
    {
        title_ = other.title_;
        content_ = other.content_;
    }

    std::unique_ptr<Document> clone() const override
    {
        return std::make_unique<SpreadsheetDocument>(*this);
    }

    void set_title(const std::string &title) override { title_ = title; }
    void set_content(const std::string &content) override
    {
        content_ = content;
    }
    std::string get_title() const override { return title_; }
    std::string get_content() const override { return content_; }
    std::string get_type() const override { return "SpreadsheetDocument"; }
    void display() const override { }

    void set_dimensions(int rows, int columns)
    {
        rows_ = rows;
        columns_ = columns;
    }

    void add_formula(const std::string &cell, const std::string &formula)
    {
        formulas_[cell] = formula;
    }

    int get_rows() const { return rows_; }
    int get_columns() const { return columns_; }
    const std::unordered_map<std::string, std::string> &get_formulas() const
    {
        return formulas_;
    }

private:
    int rows_;
    int columns_;
    std::unordered_map<std::string, std::string> formulas_;
};

// Concrete Prototype: Presentation Document
class PresentationDocument : public Document {
public:
    PresentationDocument() = default;

    PresentationDocument(const std::string &title, const std::string &content)
        : slide_count_(1)
        , theme_("Default")
    {
        title_ = title;
        content_ = content;
    }

    // Copy constructor for cloning
    PresentationDocument(const PresentationDocument &other)
        : slide_count_(other.slide_count_)
        , theme_(other.theme_)
        , slide_titles_(other.slide_titles_)
    {
        title_ = other.title_;
        content_ = other.content_;
    }

    std::unique_ptr<Document> clone() const override
    {
        return std::make_unique<PresentationDocument>(*this);
    }

    void set_title(const std::string &title) override { title_ = title; }
    void set_content(const std::string &content) override
    {
        content_ = content;
    }
    std::string get_title() const override { return title_; }
    std::string get_content() const override { return content_; }
    std::string get_type() const override { return "PresentationDocument"; }
    void display() const override { }

    void set_slide_count(int count) { slide_count_ = count; }
    void set_theme(const std::string &theme) { theme_ = theme; }
    void add_slide_title(const std::string &title)
    {
        slide_titles_.push_back(title);
    }

    int get_slide_count() const { return slide_count_; }
    std::string get_theme() const { return theme_; }
    const std::vector<std::string> &get_slide_titles() const
    {
        return slide_titles_;
    }

private:
    int slide_count_;
    std::string theme_;
    std::vector<std::string> slide_titles_;
};

// Prototype Registry/Manager
class DocumentRegistry {
public:
    void register_prototype(
        const std::string &type, std::unique_ptr<Document> prototype)
    {
        prototypes_[type] = std::move(prototype);
    }

    std::unique_ptr<Document> create_document(const std::string &type) const
    {
        auto it = prototypes_.find(type);
        if (it != prototypes_.end()) {
            return it->second->clone();
        }
        return nullptr;
    }

    std::vector<std::string> get_registered_types() const
    {
        std::vector<std::string> types;
        for (const auto &pair : prototypes_) {
            types.push_back(pair.first);
        }
        return types;
    }

    bool is_type_registered(const std::string &type) const
    {
        return prototypes_.find(type) != prototypes_.end();
    }

private:
    std::unordered_map<std::string, std::unique_ptr<Document>> prototypes_;
};

// Client code demonstrating the Prototype pattern
class DocumentFactory {
public:
    DocumentFactory()
    {
        // Register default prototypes
        auto text_proto =
            std::make_unique<TextDocument>("Template", "Enter text here");
        auto spreadsheet_proto =
            std::make_unique<SpreadsheetDocument>("Template", "Data");
        auto presentation_proto =
            std::make_unique<PresentationDocument>("Template", "Slide content");

        registry_.register_prototype("text", std::move(text_proto));
        registry_.register_prototype(
            "spreadsheet", std::move(spreadsheet_proto));
        registry_.register_prototype(
            "presentation", std::move(presentation_proto));
    }

    std::unique_ptr<Document> create_text_document()
    {
        return registry_.create_document("text");
    }

    std::unique_ptr<Document> create_spreadsheet_document()
    {
        return registry_.create_document("spreadsheet");
    }

    std::unique_ptr<Document> create_presentation_document()
    {
        return registry_.create_document("presentation");
    }

    std::unique_ptr<Document> create_document_by_type(const std::string &type)
    {
        return registry_.create_document(type);
    }

    void register_custom_prototype(
        const std::string &type, std::unique_ptr<Document> prototype)
    {
        registry_.register_prototype(type, std::move(prototype));
    }

    std::vector<std::string> get_available_types() const
    {
        return registry_.get_registered_types();
    }

private:
    DocumentRegistry registry_;
};

// Application demonstrating prototype usage
class DocumentApplication {
public:
    DocumentApplication()
        : factory_(std::make_unique<DocumentFactory>())
    {
    }

    std::unique_ptr<Document> create_new_document(const std::string &type)
    {
        return factory_->create_document_by_type(type);
    }

    std::unique_ptr<Document> duplicate_document(const Document &original)
    {
        return original.clone();
    }

    void create_document_template(
        const std::string &name, std::unique_ptr<Document> template_doc)
    {
        factory_->register_custom_prototype(name, std::move(template_doc));
    }

private:
    std::unique_ptr<DocumentFactory> factory_;
};

} // namespace t00095
} // namespace clanguml