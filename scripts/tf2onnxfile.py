import onnxruntime as rt
import numpy as np
from PIL import Image # For simple image loading and mask creation example

# --- 1. 指定你的 ONNX 模型文件路径 ---
onnx_model_path = "lama_fp32.onnx" # Make sure this is your Lama model

# --- 2. 创建 InferenceSession ---
try:
    sess = rt.InferenceSession(onnx_model_path)
    print(f"ONNX 模型 '{onnx_model_path}' 已成功加载。")
except Exception as e:
    print(f"加载 ONNX 模型失败：{e}")
    print("请检查模型路径是否正确，以及 ONNX Runtime 是否安装正确。")
    exit()

# --- 3. 获取模型的输入和输出信息 ---
input_names = [input.name for input in sess.get_inputs()]
output_names = [output.name for output in sess.get_outputs()]

print("\n--- 模型输入信息 ---")
for input in sess.get_inputs():
    print(f"名称: {input.name}, 形状: {input.shape}, 数据类型: {input.type}")

print("\n--- 模型输出信息 ---")
for output in sess.get_outputs():
    print(f"名称: {output.name}, 形状: {output.shape}, 数据类型: {output.type}")

# --- 4. 准备输入数据 (IMAGE AND MASK) ---
# For Lama Cleaner models, typically:
# - image input shape: [1, C, H, W] (e.g., [1, 3, 256, 256] for RGB)
# - mask input shape:  [1, 1, H, W] (binary mask)
# - Both usually require float32 and normalization to [0, 1]

# Example: Assuming your model expects 256x256 RGB image and mask
input_h, input_w = 512, 512
image_input_shape = [1, 3, input_h, input_w] # N, C, H, W
mask_input_shape = [1, 1, input_h, input_w]  # N, 1, H, W
input_dtype = np.float32

# Create a dummy image (replace with your actual image loading and preprocessing)
# Load an actual image and resize/preprocess it
# For demonstration, creating a random image and mask
dummy_image = np.random.rand(*image_input_shape).astype(input_dtype)

# Create a dummy mask (binary: 0 for no change, 1 for inpaint)
# Replace with your actual mask loading and preprocessing
dummy_mask = np.zeros(mask_input_shape, dtype=input_dtype)
# Example: mark a square in the mask for repair
dummy_mask[:, :, 50:100, 50:100] = 1.0 # Marking a square in the mask

# --- IMPORTANT: Ensure input names match your model's actual input names ---
# The error message implied your model expects ['mask'] and you gave ['image']
# It's common for Lama models to have inputs named 'image' and 'mask'.
# Double-check the '--- 模型输入信息 ---' output to confirm the exact names.

input_feed = {
    "image": dummy_image, # Use the actual input name for image as printed by get_inputs()
    "mask": dummy_mask    # Use the actual input name for mask as printed by get_inputs()
}

# --- 5. 执行推理 (运行模型) ---
print("\n--- 执行模型推理 ---")
try:
    results = sess.run(output_names, input_feed)
    print("模型推理成功！")

    print(f"第一个输出张量的形状: {results[0].shape}")
    print(f"第一个输出张量的前5个值: {results[0].flatten()[:5]}")

except Exception as e:
    print(f"模型推理失败：{e}")
    print("请检查输入数据是否符合模型要求。")

# --- (Rest of your code for post-processing results) ---