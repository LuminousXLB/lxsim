#include "xsi_kernel.h"

#include <cstring>
#include <dlfcn.h>
#include <filesystem>
#include <stdexcept>

#include "xsi.h"

static std::string correct_cwd(std::filesystem::path path_xsimk) {
    std::filesystem::path path_proj_root = path_xsimk.parent_path();
    while (path_proj_root.filename() != "xsim.dir") {
        if (path_proj_root.has_parent_path()) {
            path_proj_root = path_proj_root.parent_path();
        } else {
            throw std::runtime_error("xsim.dir not found");
        }
    }
    path_proj_root = path_proj_root.parent_path();
    std::filesystem::current_path(path_proj_root);

    return std::filesystem::relative(path_xsimk,
                                     std::filesystem::current_path());
}

namespace XSI {

const char *port_direction_to_string(PortDirection dir) {
    switch (dir) {
    case PortDirection::INPUT:
        return "INPUT";
    case PortDirection::OUTPUT:
        return "OUTPUT";
    case PortDirection::INOUT:
        return "INOUT";
    default:
        return "UNKNOWN";
    }
}

Kernel::Kernel(const char *xsimk, bool auto_correct_cwd) {
    auto path_xsimk = std::filesystem::path(xsimk);
    if (path_xsimk.filename() != "xsimk.so") {
        throw std::runtime_error("xsimk is not xsimk.so");
    }

    if (auto_correct_cwd) {
        correct_cwd(path_xsimk);
    }

    _dut = nullptr;
    _xsimk = dlopen(xsimk, RTLD_LAZY);
    if (!_xsimk) {
        throw std::runtime_error(dlerror());
    }
    memset(&_setup_info, 0, sizeof(_setup_info));
}

Kernel::~Kernel() {
    close();
    dlclose(_xsimk);
}

void Kernel::set_log_filename(const char *filename) {
    _setup_info.logFileName = const_cast<char *>(filename);
}

const char *Kernel::get_log_filename() const { return _setup_info.logFileName; }

void Kernel::set_wdb_filename(const char *filename) {
    _setup_info.wdbFileName = const_cast<char *>(filename);
}

const char *Kernel::get_wdb_filename() const { return _setup_info.wdbFileName; }

void Kernel::open() {
    static t_fp_xsi_open _xsi_open = (t_fp_xsi_open)dlsym(_xsimk, "xsi_open");
    if (!_xsi_open) {
        throw std::runtime_error(dlerror());
    }
    _dut = _xsi_open(&_setup_info);
}

bool Kernel::is_open() const { return _dut != nullptr; }

void Kernel::close() {
    if (_dut) {
        xsi_close(_dut);
        _dut = nullptr;
    }
}

int Kernel::get_port_number(const char *port_name) {
    int ret = xsi_get_port_number(_dut, port_name);
    if (ret < 0) {
        throw PortNotFoundException(port_name);
    }
    return ret;
}

const char *Kernel::get_port_name(int port_number) {
    const char *ret = xsi_get_port_name(_dut, port_number);
    if (ret == nullptr) {
        throw PortNotFoundException(port_number);
    }
    return ret;
}

void Kernel::put_value(int port_number, const s_xsi_vlog_logicval *value) {
    xsi_put_value(_dut, port_number, const_cast<s_xsi_vlog_logicval *>(value));
}

void Kernel::get_value(int port_number, s_xsi_vlog_logicval *value) {
    xsi_get_value(_dut, port_number, value);
}

void Kernel::run(XSI_INT64 step) { xsi_run(_dut, step); }

void Kernel::restart() { xsi_restart(_dut); }

int Kernel::get_num_ports() const { return xsi_get_int(_dut, xsiNumTopPorts); }

int Kernel::get_time_precision() const {
    return xsi_get_int(_dut, xsiTimePrecisionKernel);
}

PortDirection Kernel::get_port_direction(int port_number) const {
    return (PortDirection)xsi_get_int_port(_dut, port_number,
                                           xsiDirectionTopPort);
}

unsigned Kernel::get_port_width(int port_number) const {
    return xsi_get_int_port(_dut, port_number, xsiHDLValueSize);
}

Status Kernel::get_status() { return (Status)xsi_get_status(_dut); }

const char *Kernel::get_error_info() { return xsi_get_error_info(_dut); }

void Kernel::trace_all() { xsi_trace_all(_dut); }

uint64_t Kernel::get_time() { return xsi_get_time(_dut); }

} // namespace XSI
