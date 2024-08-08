module;

#include <array>
#include <map>
#include <memory>
#include <string>
#include <vector>

export module t30015.app;
import t30015.lib1;

export namespace clanguml::t30015 {

class CBA : public CF {
public:
    CA *ca_;
    CB<int> cb_;
    std::shared_ptr<CC> cc_;
    std::map<std::string, std::unique_ptr<CD>> *cd_;
    std::array<CO, 5> co_;
    static CP *cp_;

    CBA() = default;

    CBA(CN *cn) { }

    friend CR;

    template <typename... Item> CBA(std::tuple<Item...> &items) { }

    void ce(const std::vector<CE> /*ce_*/) { }

    std::shared_ptr<CG> cg() { return {}; }

    template <typename T> void ch(std::map<T, std::shared_ptr<CH>> &ch_) { }

    template <typename T> std::map<T, std::shared_ptr<CI>> ci(T * /*t*/)
    {
        return {};
    }

    S s;
};

void cj(std::unique_ptr<CJ> /*cj_*/) { }

std::unique_ptr<CK> ck() { return {}; }

template <typename T> void cl(std::map<T, std::shared_ptr<CL>> & /*ch_*/) { }

template <typename T> std::map<T, std::shared_ptr<CM>> cm() { return {}; }

} // namespace clanguml::t30013