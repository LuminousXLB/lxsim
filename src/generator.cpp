#include <cstddef>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

#include "xsi.h"
#include "xsi_kernel.h"

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <xsimk.so>" << std::endl;
        return 1;
    }

    auto dut = XSI::Kernel::create(argv[1], true);

    dut->open();

    puts("#include \"xsi_kernel.h\"");
    puts("struct DUT{");
    puts("\tstd::shared_ptr<XSI::Kernel> kernel;");
    for (int pn = 0; pn < dut->get_num_ports(); pn++) {
        const char *dir =
            XSI::port_direction_to_string(dut->get_port_direction(pn));
        const char *name = dut->get_port_name(pn);
        int width = (dut->get_port_width(pn) + 31) / 32;
        printf("\tXSI::Port<XSI::%s,%d> %s;\n", dir, width, name);
    }

    printf("\tDUT(std::shared_ptr<XSI::Kernel> kernel)");
    printf(":\n\t\tkernel(kernel)");
    for (int pn = 0; pn < dut->get_num_ports(); pn++) {
        printf(",\n\t\t%s(kernel,\"%s\" , %d, %d)", dut->get_port_name(pn),
               dut->get_port_name(pn), pn, dut->get_port_width(pn));
    }
    puts("{}");
    puts("};");

    dut->close();
}
