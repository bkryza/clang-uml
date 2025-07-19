#include <memory>
#include <string>
#include <vector>

namespace clanguml {
namespace t00094 {

// Builder pattern implementation: Computer system configuration

// Product - the complex object to be built
class Computer {
public:
    void set_cpu(const std::string &cpu) { cpu_ = cpu; }
    void set_memory(int memory_gb) { memory_gb_ = memory_gb; }
    void set_storage(const std::string &storage) { storage_ = storage; }
    void set_graphics_card(const std::string &gpu) { graphics_card_ = gpu; }
    void set_motherboard(const std::string &motherboard) { motherboard_ = motherboard; }
    void add_port(const std::string &port) { ports_.push_back(port); }
    void set_power_supply(int watts) { power_supply_watts_ = watts; }
    void set_case_type(const std::string &case_type) { case_type_ = case_type; }
    
    const std::string &get_cpu() const { return cpu_; }
    int get_memory() const { return memory_gb_; }
    const std::string &get_storage() const { return storage_; }
    const std::string &get_graphics_card() const { return graphics_card_; }
    const std::string &get_motherboard() const { return motherboard_; }
    const std::vector<std::string> &get_ports() const { return ports_; }
    int get_power_supply() const { return power_supply_watts_; }
    const std::string &get_case_type() const { return case_type_; }
    
    void display_specs() const { }

private:
    std::string cpu_;
    int memory_gb_{0};
    std::string storage_;
    std::string graphics_card_;
    std::string motherboard_;
    std::vector<std::string> ports_;
    int power_supply_watts_{0};
    std::string case_type_;
};

// Abstract Builder interface
class ComputerBuilder {
public:
    virtual ~ComputerBuilder() = default;
    
    virtual void build_cpu() = 0;
    virtual void build_memory() = 0;
    virtual void build_storage() = 0;
    virtual void build_graphics_card() = 0;
    virtual void build_motherboard() = 0;
    virtual void build_ports() = 0;
    virtual void build_power_supply() = 0;
    virtual void build_case() = 0;
    
    virtual std::unique_ptr<Computer> get_computer() = 0;
    virtual void reset() = 0;

protected:
    std::unique_ptr<Computer> computer_;
};

// Concrete Builder for Gaming Computer
class GamingComputerBuilder : public ComputerBuilder {
public:
    GamingComputerBuilder() { reset(); }
    
    void build_cpu() override
    {
        computer_->set_cpu("Intel Core i9-13900K");
    }
    
    void build_memory() override
    {
        computer_->set_memory(32);
    }
    
    void build_storage() override
    {
        computer_->set_storage("1TB NVMe SSD");
    }
    
    void build_graphics_card() override
    {
        computer_->set_graphics_card("NVIDIA RTX 4080");
    }
    
    void build_motherboard() override
    {
        computer_->set_motherboard("ASUS ROG Maximus Z790");
    }
    
    void build_ports() override
    {
        computer_->add_port("USB 3.2");
        computer_->add_port("USB-C");
        computer_->add_port("HDMI 2.1");
        computer_->add_port("DisplayPort 1.4");
        computer_->add_port("Ethernet");
        computer_->add_port("Audio Jack");
    }
    
    void build_power_supply() override
    {
        computer_->set_power_supply(850);
    }
    
    void build_case() override
    {
        computer_->set_case_type("Full Tower RGB");
    }
    
    std::unique_ptr<Computer> get_computer() override
    {
        auto result = std::move(computer_);
        reset();
        return result;
    }
    
    void reset() override
    {
        computer_ = std::make_unique<Computer>();
    }
};

// Concrete Builder for Office Computer
class OfficeComputerBuilder : public ComputerBuilder {
public:
    OfficeComputerBuilder() { reset(); }
    
    void build_cpu() override
    {
        computer_->set_cpu("Intel Core i5-13400");
    }
    
    void build_memory() override
    {
        computer_->set_memory(16);
    }
    
    void build_storage() override
    {
        computer_->set_storage("512GB SSD");
    }
    
    void build_graphics_card() override
    {
        computer_->set_graphics_card("Integrated Graphics");
    }
    
    void build_motherboard() override
    {
        computer_->set_motherboard("MSI Pro B760M");
    }
    
    void build_ports() override
    {
        computer_->add_port("USB 3.0");
        computer_->add_port("USB 2.0");
        computer_->add_port("HDMI");
        computer_->add_port("VGA");
        computer_->add_port("Ethernet");
        computer_->add_port("Audio Jack");
    }
    
    void build_power_supply() override
    {
        computer_->set_power_supply(450);
    }
    
    void build_case() override
    {
        computer_->set_case_type("Mini Tower");
    }
    
    std::unique_ptr<Computer> get_computer() override
    {
        auto result = std::move(computer_);
        reset();
        return result;
    }
    
    void reset() override
    {
        computer_ = std::make_unique<Computer>();
    }
};

// Director - orchestrates the building process
class ComputerDirector {
public:
    explicit ComputerDirector(std::unique_ptr<ComputerBuilder> builder)
        : builder_(std::move(builder))
    {
    }
    
    void set_builder(std::unique_ptr<ComputerBuilder> builder)
    {
        builder_ = std::move(builder);
    }
    
    std::unique_ptr<Computer> construct_basic_computer()
    {
        builder_->reset();
        builder_->build_cpu();
        builder_->build_memory();
        builder_->build_storage();
        builder_->build_motherboard();
        builder_->build_power_supply();
        builder_->build_case();
        return builder_->get_computer();
    }
    
    std::unique_ptr<Computer> construct_full_computer()
    {
        builder_->reset();
        builder_->build_cpu();
        builder_->build_memory();
        builder_->build_storage();
        builder_->build_graphics_card();
        builder_->build_motherboard();
        builder_->build_ports();
        builder_->build_power_supply();
        builder_->build_case();
        return builder_->get_computer();
    }
    
    std::unique_ptr<Computer> construct_custom_computer()
    {
        builder_->reset();
        builder_->build_cpu();
        builder_->build_memory();
        builder_->build_storage();
        builder_->build_graphics_card();
        builder_->build_motherboard();
        return builder_->get_computer();
    }

private:
    std::unique_ptr<ComputerBuilder> builder_;
};

// Client code demonstrating the Builder pattern
class ComputerStore {
public:
    std::unique_ptr<Computer> order_gaming_computer()
    {
        auto builder = std::make_unique<GamingComputerBuilder>();
        ComputerDirector director(std::move(builder));
        return director.construct_full_computer();
    }
    
    std::unique_ptr<Computer> order_office_computer()
    {
        auto builder = std::make_unique<OfficeComputerBuilder>();
        ComputerDirector director(std::move(builder));
        return director.construct_full_computer();
    }
    
    std::unique_ptr<Computer> order_custom_computer(std::unique_ptr<ComputerBuilder> builder)
    {
        ComputerDirector director(std::move(builder));
        return director.construct_custom_computer();
    }
};

} // namespace t00094
} // namespace clanguml