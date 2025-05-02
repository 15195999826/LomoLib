import sys
import subprocess
import pkg_resources

# 检查所需的库
required_packages = ['pandas', 'openpyxl', 'pypinyin']

# 输出Python版本
print(f"Python版本: {sys.version}")

# 检查已安装的库
installed_packages = {pkg.key for pkg in pkg_resources.working_set}
missing_packages = [pkg for pkg in required_packages if pkg not in installed_packages]

if missing_packages:
    print(f"缺少以下库: {', '.join(missing_packages)}")
    
    # 尝试安装缺少的库
    print("正在尝试安装缺少的库...")
    for package in missing_packages:
        try:
            subprocess.check_call([sys.executable, "-m", "pip", "install", package])
            print(f"成功安装 {package}")
        except Exception as e:
            print(f"安装 {package} 失败: {str(e)}")
else:
    print("所有必需的库都已安装")

# 打印已安装库的版本
print("\n已安装库版本:")
for package in required_packages:
    try:
        version = pkg_resources.get_distribution(package).version
        print(f"{package}: {version}")
    except pkg_resources.DistributionNotFound:
        print(f"{package}: 未安装") 