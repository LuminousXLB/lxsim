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
    puts("\tstd::shared_ptr<XSI::Kernel> _kernel;");
    for (int pn = 0; pn < dut->get_num_ports(); pn++) {
        printf("\tXSI::Port<XSI::%s,%d> %s;\n",
               XSI::port_direction_to_string(dut->get_port_direction(pn)),
               dut->get_port_width(pn), dut->get_port_name(pn));
    }

    printf("\tDUT(const char *xsimk, bool auto_correct_cwd)");
    printf(":\n\t _kernel(XSI::Kernel::create(xsimk, auto_correct_cwd))");
    for (int pn = 0; pn < dut->get_num_ports(); pn++) {
        printf(",\n\t %s(_kernel,\"%s\" , %d, %d)", dut->get_port_name(pn),
               dut->get_port_name(pn), pn, dut->get_port_width(pn));
    }
    puts("{}");
    puts("};");

    dut->close();
}
