file ./src/lab1/vmlinux

# 启用完整的打印信息，包括结构体和类的成员变量
set print pretty on

b start_kernel

# 设置自定义GDB命令
define mycommand
    # 自定义命令逻辑
end

# 在这里可以添加更多的自定义命令和配置
target remote:1234