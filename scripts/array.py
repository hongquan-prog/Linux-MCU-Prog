import sys  
import os  
  
def bin_to_c_array(input_file, output_file):  
    # 读取二进制文件内容  
    with open(input_file, 'rb') as f:  
        content = f.read()  
      
    # 获取不带路径的文件名，并替换非法字符  
    file_name = os.path.basename(input_file).replace('.', '_').replace('-', '_').replace(' ', '_')  
      
    # 数组名和长度变量名  
    array_name = file_name  
    length_name = f"{file_name}_len"  
      
    # 生成C语言数组内容  
    array_content = []  
    for i in range(0, len(content), 12):  
        chunk = content[i:i+12]  
        array_line = f"  {', '.join(f'0x{b:02x}' for b in chunk)},"  
        array_content.append(array_line)  
      
    # 写入到输出文件  
    with open(output_file, 'w') as f:  
        f.write(f"static const unsigned char s_{array_name}[] = {{\n")  
        f.write('\n'.join(array_content))  
        f.write(f"\n}};\n\n")  
        f.write(f"static const unsigned int s_{length_name} = {len(content)};\n\n")
        f.write(f"const unsigned char* {array_name}_ptr() \n{{\n    return s_{array_name};\n}}\n\n")  
        f.write(f"unsigned int {array_name}_len() \n{{\n    return s_{length_name};\n}}\n\n")  
  
if __name__ == "__main__":  
    if len(sys.argv) != 3:  
        print(f"Usage: python3 {sys.argv[0]} <input_bin_file> <output_c_file>")  
        sys.exit(1)  
      
    input_file = sys.argv[1]  
    output_file = sys.argv[2]  
      
    bin_to_c_array(input_file, output_file)