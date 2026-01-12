# Pack Models Tool

模型序列化打包工具，用于将不同类型的AI模型序列化为统一格式并打包管理。

## 功能特性

- **模型序列化**: 支持MNN和OpenVINO模型格式的序列化
- **模型打包**: 将多个模型文件打包为单一压缩包
- **跨平台支持**: 兼容Linux和Windows平台
- **压缩存储**: 使用zlib压缩减少存储空间
- **完整性校验**: CRC32校验确保数据完整性
- **元数据管理**: 支持包信息和自定义元数据
- **安全加密**: 使用AES-256-GCM算法对模型和包进行加密保护
- **用户无感**: 自动密钥管理，加解密过程对用户完全透明

## 编译要求

- CMake 3.16+
- C++17 编译器
- zlib 库
- CryptoPP 库 (用于加密功能)
- RayShape kernel 库

## 编译方法

```bash
# 在项目根目录下
mkdir build && cd build
cmake .. -DENABLE_PACK_MODELS_TOOL=ON
make pack_models

# 或者直接在tools目录下编译
cd tools/pack_models
mkdir build && cd build
cmake ..
make
```

## 使用方法

### 序列化模型

#### MNN模型序列化
```bash
# 普通序列化
./pack_models serialize -t mnn -i model.mnn --config config.json -o model_serialized.rsm

# 自动加密序列化
./pack_models serialize -t mnn -i model.mnn --config config.json -o model_serialized.rsm --encrypt
```

#### OpenVINO模型序列化
```bash
# 普通序列化
./pack_models serialize -t openvino --xml model.xml --bin model.bin --config config.json -o model_serialized.rsm

# 自动加密序列化
./pack_models serialize -t openvino --xml model.xml --bin model.bin --config config.json -o model_serialized.rsm --encrypt
```

#### ONNX模型序列化
```bash
# 普通序列化
./pack_models serialize -t onnx -i model.onnx --config config.json -o model_serialized.rsm

# 自动加密序列化
./pack_models serialize -t onnx -i model.onnx --config config.json -o model_serialized.rsm --encrypt
```

### 创建模型包

```bash
# 基本打包
./pack_models package -n "My Model Package" --version "1.0.0" \
  -d "示例模型包" --files model1.rsm model2.rsm -o package.rsmp

# 自动加密打包
./pack_models package -n "Secure Model Package" --version "1.0.0" \
  -d "加密模型包" --files model1.rsm model2.rsm -o package.rsmp --encrypt

# 带自定义元数据的自动加密打包
./pack_models package -n "Advanced Model" --version "2.0" \
  --files model.rsm config.json --metadata "author=RayShape,license=MIT" \
  -o advanced_package.rsmp -c 9 --encrypt
```

### 提取模型包

```bash
# 提取包（自动检测和处理加密）
./pack_models extract -i package.rsmp -o extracted_models/

# 加密包也是自动解密，无需额外参数
./pack_models extract -i encrypted_package.rsmp -o extracted_models/
```

### 查看包信息

```bash
# 列出包内容（自动处理加密）
./pack_models list -i package.rsmp

# 显示包详细信息（自动处理加密）
./pack_models info -i package.rsmp

# 验证包完整性（自动处理加密）
./pack_models validate -i package.rsmp
```

## 命令参考

### 全局选项

- `-h, --help`: 显示帮助信息
- `-v, --verbose`: 启用详细输出
- `-f, --force`: 强制覆盖现有文件
- `-c, --compression <0-9>`: 设置压缩级别 (默认: 6)

### 加密选项

- `--encrypt`: 启用自动加密功能（加解密完全透明，无需密码管理）

### 支持的模型类型

- `mnn`: MNN模型格式
- `openvino`: OpenVINO模型格式 (需要XML和BIN文件)
- `onnx`: ONNX模型格式

### 命令详解

#### serialize - 序列化模型
将原始模型文件序列化为统一的二进制格式。

**必需参数:**
- `-t, --type <type>`: 模型类型
- `-i, --input <path>`: 输入模型文件
- `-o, --output <path>`: 输出序列化文件

**OpenVINO特殊参数:**
- `--xml <path>`: XML配置文件
- `--bin <path>`: 二进制权重文件

**必需参数:**
- `--config <path>`: 模型配置JSON文件，用于保存模型解析得到的配置信息

#### package - 创建包
将多个文件打包为压缩的模型包。

**必需参数:**
- `-n, --name <name>`: 包名称
- `--files <files...>`: 输入文件列表
- `-o, --output <path>`: 输出包文件

**可选参数:**
- `--version <version>`: 包版本 (默认: 1.0.0)
- `-d, --description <desc>`: 包描述
- `--metadata <key=val,...>`: 自定义元数据

#### extract - 提取包
从模型包中提取所有文件。

**必需参数:**
- `-i, --input <path>`: 输入包文件
- `-o, --output <path>`: 输出目录

#### list - 列出内容
显示包中的文件列表。

**必需参数:**
- `-i, --input <path>`: 输入包文件

#### info - 显示信息
显示包的详细信息包括元数据。

**必需参数:**
- `-i, --input <path>`: 输入包文件

#### validate - 验证包
检查包的完整性和有效性。

**必需参数:**
- `-i, --input <path>`: 输入包文件

## 包格式规范

模型包使用自定义的RSMP (RayShape Model Package) 格式：

```
[4字节] 魔数: "RSMP"
[4字节] 版本号
[4字节] 元数据大小
[4字节] 元数据CRC32
[变长]  压缩的元数据 (JSON格式)
[4字节] 文件数量
对于每个文件:
  [4字节] 文件名长度
  [变长]  文件名
  [4字节] 原始大小
  [4字节] 压缩大小
  [4字节] 文件CRC32
  [变长]  压缩的文件内容
```

## 使用示例

### 完整工作流程

```bash
# 1. 序列化MNN模型（启用自动加密）
./pack_models serialize -t mnn -i yolo.mnn --config yolo_config.json -o yolo_serialized.rsm --encrypt -v

# 2. 创建包含多个文件的加密模型包
./pack_models package -n "YOLO Detection Model" --version "3.0" \
  -d "YOLOv5目标检测模型" \
  --files yolo_serialized.rsm config.yaml labels.txt \
  --metadata "framework=MNN,task=detection,input_size=640x640" \
  -o yolo_package.rsmp -c 9 --encrypt -v

# 3. 验证包（自动处理加密）
./pack_models validate -i yolo_package.rsmp

# 4. 查看包信息（自动处理加密）
./pack_models info -i yolo_package.rsmp

# 5. 提取到新位置（自动处理加密）
./pack_models extract -i yolo_package.rsmp -o deployed_model/ -v
```

### 批量处理脚本示例

```bash
#!/bin/bash
# 批量序列化MNN模型（启用自动加密）

for model in *.mnn; do
    echo "Serializing $model with auto-encryption..."
    config_file="${model%.mnn}_config.json"
    ./pack_models serialize -t mnn -i "$model" --config "$config_file" \
      -o "${model%.mnn}_serialized.rsm" --encrypt
done

echo "Creating encrypted batch package..."
./pack_models package -n "Batch Models" --version "1.0" \
  --files *_serialized.rsm -o batch_models.rsmp --encrypt
```

## 错误处理

工具提供详细的错误信息和返回码：

- `0`: 成功
- `1`: 一般错误 (参数错误、文件不存在等)
- 其他: 特定错误码

使用 `-v` 选项获取详细的执行信息和调试输出。

## 加密安全性

### 加密算法
- **对称加密**: AES-256-GCM (Galois/Counter Mode)
- **密钥派生**: PBKDF2 (Password-Based Key Derivation Function 2)
- **认证**: GCM提供内建的认证和完整性校验
- **随机性**: 使用CryptoPP的AutoSeededRandomPool生成加密安全的随机数

### 安全特性
- **前向安全**: 每次加密使用不同的随机IV (初始化向量)
- **密钥强度**: 支持256位密钥长度，提供军用级加密强度
- **完整性保护**: GCM模式提供认证标签，防止数据篡改
- **密码验证**: 强制密码复杂度要求 (最少8位，包含数字或特殊字符)

### 自动加密特性
1. **透明加密**: 启用`--encrypt`选项后，加密完全自动化，用户无需管理密码
2. **内容绑定**: 加密密钥基于文件内容哈希生成，相同内容在任何平台都可自动解密
3. **安全可靠**: 使用军用级AES-256-GCM算法，安全性不会因为简化而降低
4. **跨平台**: 加密文件可在不同操作系统、不同机器间正常解密，具有完全可移植性

## 加密原理说明

### 内容绑定加密机制
```
1. 原始模型文件 → SHA256哈希 → 内容指纹
2. 内容指纹 → 密钥生成 → AES-256密钥
3. 原始数据 + 密钥 → AES-256-GCM加密 → 加密数据
4. 加密文件格式：[魔数][版本][内容指纹][加密数据]
```

### 解密过程
```
1. 读取加密文件 → 提取内容指纹
2. 内容指纹 → 重新生成密钥
3. 密钥 + 加密数据 → AES-256-GCM解密 → 原始数据
```

**优势**: 相同内容的文件在任何平台都使用相同密钥，实现完全的跨平台兼容性。

## 技术实现

- **序列化**: 基于cereal库实现多态模型序列化，支持模型配置信息的完整保存
- **配置文件**: 支持将模型解析得到的JSON配置信息一起序列化，确保模型的完整性
- **压缩**: 使用zlib进行数据压缩
- **校验**: CRC32确保数据完整性
- **加密**: 基于CryptoPP库实现AES-256-GCM加密，提供军用级安全保护
- **跨平台**: 使用标准C++17和CMake构建系统
- **内存管理**: RAII和智能指针确保内存安全
- **线程安全**: 加密操作支持多线程环境

## 许可证

Copyright (c) 2025 Shenzhen RayShape Medical Technology Co., Ltd.
