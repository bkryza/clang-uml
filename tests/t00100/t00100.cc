#include <concepts>
#include <memory>
#include <string>
#include <vector>

namespace clanguml {
namespace t00100 {

// Builder pattern implementation using static polymorphism with C++20 concepts

// Product - the complex object to be built
class Computer {
public:
    void set_cpu(const std::string &cpu) { cpu_ = cpu; }
    void set_memory(int memory_gb) { memory_gb_ = memory_gb; }
    void set_storage(const std::string &storage) { storage_ = storage; }
    void set_graphics_card(const std::string &gpu) { graphics_card_ = gpu; }
    void set_motherboard(const std::string &motherboard)
    {
        motherboard_ = motherboard;
    }
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

// C++20 concept for Builder interface using static polymorphism
template <typename T>
concept ComputerBuilderConcept = requires(T t, Computer &computer) {
    { t.build_cpu(computer) } -> std::same_as<void>;
    { t.build_memory(computer) } -> std::same_as<void>;
    { t.build_storage(computer) } -> std::same_as<void>;
    { t.build_graphics_card(computer) } -> std::same_as<void>;
    { t.build_motherboard(computer) } -> std::same_as<void>;
    { t.build_ports(computer) } -> std::same_as<void>;
    { t.build_power_supply(computer) } -> std::same_as<void>;
    { t.build_case(computer) } -> std::same_as<void>;
};

// Template base for concrete builders
template <typename Derived> class ComputerBuilderBase {
public:
    std::unique_ptr<Computer> get_computer()
    {
        auto result = std::move(computer_);
        reset();
        return result;
    }

    Computer &get_computer_ref() { return *computer_; }

    void reset() { computer_ = std::make_unique<Computer>(); }

protected:
    std::unique_ptr<Computer> computer_;

    ComputerBuilderBase() { reset(); }
};

// Concrete Builder for Gaming Computer using static polymorphism
class GamingComputerBuilder
    : public ComputerBuilderBase<GamingComputerBuilder> {
public:
    void build_cpu(Computer &computer)
    {
        computer.set_cpu("Intel Core i9-13900K");
    }

    void build_memory(Computer &computer) { computer.set_memory(32); }

    void build_storage(Computer &computer)
    {
        computer.set_storage("1TB NVMe SSD");
    }

    void build_graphics_card(Computer &computer)
    {
        computer.set_graphics_card("NVIDIA RTX 4080");
    }

    void build_motherboard(Computer &computer)
    {
        computer.set_motherboard("ASUS ROG Maximus Z790");
    }

    void build_ports(Computer &computer)
    {
        computer.add_port("USB 3.2");
        computer.add_port("USB-C");
        computer.add_port("HDMI 2.1");
        computer.add_port("DisplayPort 1.4");
        computer.add_port("Ethernet");
        computer.add_port("Audio Jack");
    }

    void build_power_supply(Computer &computer)
    {
        computer.set_power_supply(850);
    }

    void build_case(Computer &computer)
    {
        computer.set_case_type("Full Tower RGB");
    }
};

// Concrete Builder for Office Computer using static polymorphism
class OfficeComputerBuilder
    : public ComputerBuilderBase<OfficeComputerBuilder> {
public:
    void build_cpu(Computer &computer)
    {
        computer.set_cpu("Intel Core i5-13400");
    }

    void build_memory(Computer &computer) { computer.set_memory(16); }

    void build_storage(Computer &computer)
    {
        computer.set_storage("512GB SSD");
    }

    void build_graphics_card(Computer &computer)
    {
        computer.set_graphics_card("Integrated Graphics");
    }

    void build_motherboard(Computer &computer)
    {
        computer.set_motherboard("MSI Pro B760M");
    }

    void build_ports(Computer &computer)
    {
        computer.add_port("USB 3.0");
        computer.add_port("USB 2.0");
        computer.add_port("HDMI");
        computer.add_port("VGA");
        computer.add_port("Ethernet");
        computer.add_port("Audio Jack");
    }

    void build_power_supply(Computer &computer)
    {
        computer.set_power_supply(450);
    }

    void build_case(Computer &computer)
    {
        computer.set_case_type("Mini Tower");
    }
};

// Director template using static polymorphism and C++20 concepts
template <ComputerBuilderConcept BuilderType> class ComputerDirector {
public:
    explicit ComputerDirector(BuilderType &builder)
        : builder_(builder)
    {
    }

    std::unique_ptr<Computer> construct_basic_computer()
    {
        builder_.reset();
        auto &computer = builder_.get_computer_ref();
        builder_.build_cpu(computer);
        builder_.build_memory(computer);
        builder_.build_storage(computer);
        builder_.build_motherboard(computer);
        builder_.build_power_supply(computer);
        builder_.build_case(computer);
        return builder_.get_computer();
    }

    std::unique_ptr<Computer> construct_full_computer()
    {
        builder_.reset();
        auto &computer = builder_.get_computer_ref();
        builder_.build_cpu(computer);
        builder_.build_memory(computer);
        builder_.build_storage(computer);
        builder_.build_graphics_card(computer);
        builder_.build_motherboard(computer);
        builder_.build_ports(computer);
        builder_.build_power_supply(computer);
        builder_.build_case(computer);
        return builder_.get_computer();
    }

    std::unique_ptr<Computer> construct_custom_computer()
    {
        builder_.reset();
        auto &computer = builder_.get_computer_ref();
        builder_.build_cpu(computer);
        builder_.build_memory(computer);
        builder_.build_storage(computer);
        builder_.build_graphics_card(computer);
        builder_.build_motherboard(computer);
        return builder_.get_computer();
    }

private:
    BuilderType &builder_;
};

// Client code demonstrating the Builder pattern with static polymorphism
class ComputerStore {
public:
    std::unique_ptr<Computer> order_gaming_computer()
    {
        GamingComputerBuilder builder;
        ComputerDirector director(builder);
        return director.construct_full_computer();
    }

    std::unique_ptr<Computer> order_office_computer()
    {
        OfficeComputerBuilder builder;
        ComputerDirector director(builder);
        return director.construct_full_computer();
    }

    template <ComputerBuilderConcept BuilderType>
    std::unique_ptr<Computer> order_custom_computer(BuilderType &builder)
    {
        ComputerDirector director(builder);
        return director.construct_custom_computer();
    }
};

} // namespace t00100
} // namespace clanguml