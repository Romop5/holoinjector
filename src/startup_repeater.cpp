#include "startup_enhancer.hpp"
#include "repeater.hpp"

__attribute((constructor)) void repeater_setup()
{
    puts("Repeater startup");
    enhancer_setup(std::make_unique<ve::Repeater>());
}

__attribute((destructor)) void repeater_cleaner()
{
    enhancer_cleaner();
}
