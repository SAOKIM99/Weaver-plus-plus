# Tổng Quan Hệ Thống

## 🚲 Hệ Thống Xe Đạp Thông Minh Weaver++

Weaver++ là một hệ thống xe đạp thông minh được thiết kế để tích hợp nhiều thành phần phần cứng và phần mềm nhằm nâng cao bảo mật, giám sát hiệu năng và trải nghiệm người dùng. Tài liệu này cung cấp tổng quan về kiến trúc hệ thống, các tính năng chính và luồng hoạt động.

---

## 🛠️ Kiến Trúc Hệ Thống

### **1. Thành Phần Phần Cứng**
- **Vi Điều Khiển ESP32**: Bộ xử lý hai nhân cho BLE, CAN và quản lý cảm biến.
- **Mô-đun BLE**: Bluetooth tích hợp trên ESP32 để bonding, pairing và giao tiếp mã hóa.
- **Đầu Đọc RFID**: Dùng để xác thực người dùng.
- **CAN Bus**: Giao tiếp với hệ thống quản lý pin (BMS) và bộ điều khiển động cơ.
- **Cảm Biến**: Đo tốc độ, nhiệt độ và các dữ liệu môi trường khác.
- **Màn Hình**: Giao diện người dùng để hiển thị dữ liệu thời gian thực.

### **2. Thành Phần Phần Mềm**
- **Nhiệm Vụ FreeRTOS**:
  - Nhiệm Vụ BLE: Xử lý bonding, pairing và giao tiếp.
  - Nhiệm Vụ RFID: Quản lý xác thực RFID.
  - Nhiệm Vụ Cảm Biến: Đọc và xử lý dữ liệu cảm biến.
  - Nhiệm Vụ CAN: Giao tiếp với BMS và bộ điều khiển.
  - Nhiệm Vụ Hiển Thị: Cập nhật giao diện người dùng.
- **NimBLE Stack**: Thư viện BLE nhẹ cho kết nối an toàn.
- **PlatformIO**: Môi trường xây dựng và triển khai.

---

## 🔒 Tính Năng Bảo Mật

### **1. BLE Bonding**
- **Mã Hóa**: Mã hóa AES-128 để giao tiếp an toàn.
- **Bảo Vệ MITM**: Ngăn chặn tấn công man-in-the-middle.
- **Địa Chỉ Ngẫu Nhiên (RPA)**: Đảm bảo quyền riêng tư của người dùng.
- **Giới Hạn Thiết Bị Bonded**: Tối đa 5 thiết bị với cơ chế tự động dọn dẹp.

### **2. Xác Thực Kép**
- **Nút BOOT**: Nút vật lý để xác nhận thủ công.
- **Thẻ RFID**: Xác thực không tiếp xúc để pairing và mở khóa.

### **3. Bảo Toàn Dữ Liệu**
- **Giao Thức CAN**: Giao tiếp đáng tin cậy với kiểm tra lỗi.
- **Xác Thực Cảm Biến**: Đảm bảo dữ liệu chính xác.

---

## ⚙️ Luồng Hoạt Động

### **1. Khởi Tạo Hệ Thống**
- ESP32 khởi động và khởi tạo tất cả các thành phần.
- BLE bắt đầu quảng bá để thiết bị có thể tìm thấy.
- Màn hình hiển thị trạng thái hệ thống.

### **2. Ghép Nối Thiết Bị**
- Người dùng kết nối qua BLE.
- PIN được hiển thị trên Serial Monitor.
- Người dùng xác nhận pairing qua nút BOOT hoặc thẻ RFID.
- Thiết bị được bond và lưu vào Flash.

### **3. Hoạt Động Bình Thường**
- BLE: Giao tiếp mã hóa với các thiết bị đã bond.
- RFID: Kiểm tra xác thực liên tục.
- Cảm Biến: Thu thập và xử lý dữ liệu thời gian thực.
- CAN: Cập nhật định kỳ đến/từ BMS và bộ điều khiển.
- Màn Hình: Cập nhật trực tiếp tốc độ, pin và cảnh báo.

### **4. Quản Lý Thiết Bị Bonded**
- Tối đa 5 thiết bị bonded.
- Thiết bị cũ nhất bị xóa khi vượt quá giới hạn.
- Reset thủ công qua hàm `clearBondedDevices()`.

---

## 📊 Chỉ Số Hiệu Năng

| Thành Phần       | Tần Suất Cập Nhật | Ghi Chú                          |
|------------------|-------------------|----------------------------------|
| BLE              | 20 Hz            | Bonding, pairing, đồng bộ dữ liệu|
| RFID             | 10 Hz            | Kiểm tra xác thực                |
| Cảm Biến         | 5 Hz             | Tốc độ, nhiệt độ, v.v.           |
| CAN Bus          | 2 Hz             | Cập nhật BMS và bộ điều khiển    |
| Màn Hình         | 0.2 Hz           | Làm mới giao diện người dùng     |

---

## 🧪 Danh Sách Kiểm Tra Kiểm Thử

- [ ] Bond 5 thiết bị qua BLE.
- [ ] Xác minh tự động xóa thiết bị cũ nhất.
- [ ] Kiểm tra xác thực kép (BOOT + RFID).
- [ ] Xác nhận giao tiếp mã hóa.
- [ ] Kiểm tra độ chính xác của cảm biến.
- [ ] Xác nhận tính toàn vẹn của giao tiếp CAN.
- [ ] Đảm bảo màn hình cập nhật chính xác.

---

## 📅 Lịch Sử Phiên Bản

- **v2.0** (10/10/2025): Thêm giới hạn thiết bị bonded và xác thực kép.
- **v1.0** (09/10/2025): Phát hành ban đầu với BLE bonding và tích hợp CAN.

---

**Dự Án**: Weaver++ (+ SAO KIM +)  
**Tác Giả**: SAOKIM99  
**Cập Nhật Lần Cuối**: 10/10/2025