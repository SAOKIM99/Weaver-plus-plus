# Weaver++

Weaver++ là một hệ thống xe đạp thông minh được thiết kế để nâng cao bảo mật, giám sát hiệu năng và cải thiện trải nghiệm người dùng. Dự án tích hợp các công nghệ hiện đại như BLE, RFID, CAN Bus và cảm biến để cung cấp một giải pháp toàn diện cho xe đạp điện.

---

## Hoạt động của hệ thống main và display

### 1. Hệ thống `main` (main logic)
- Khởi tạo phần cứng: Thiết lập các chân GPIO cho các chức năng như khóa xe, phanh, xi-nhan, cảm biến Hall, v.v.
- Khởi tạo các module:
	- **BLE**: Quản lý kết nối Bluetooth, bảo mật, và liên kết thiết bị.
	- **RFID**: Quản lý xác thực thẻ RFID để mở/khóa xe.
	- **BMS & VESC**: Giao tiếp với các module quản lý pin (BMS) và điều khiển động cơ (VESC) qua UART/CAN.
	- **Sensor**: Đọc dữ liệu cảm biến như tốc độ, trạng thái phanh, xi-nhan, v.v.
- Vòng lặp chính:
	- Cập nhật trạng thái cảm biến, BMS, VESC, RFID, BLE.
	- Xử lý các sự kiện như mở/khóa xe, thay đổi trạng thái vận hành, gửi/nhận dữ liệu qua CAN.
	- Gửi dữ liệu trạng thái xe đến module hiển thị (display).

### 2. Hệ thống `display` (BikeDisplayUI)
- Khởi tạo giao diện: Sử dụng thư viện LVGL để tạo các thành phần UI như đồng hồ tốc độ, hiển thị pin, nhiệt độ động cơ, trạng thái Bluetooth, xi-nhan, v.v.
- Cập nhật dữ liệu:
	- Nhận dữ liệu từ hệ thống chính (main) qua CAN hoặc các hàm cập nhật.
	- Hiển thị tốc độ, dòng điện, phần trăm pin, nhiệt độ ECU/động cơ, trạng thái Bluetooth, xi-nhan, quãng đường, v.v.
	- Đổi màu các thành phần UI theo ngưỡng cảnh báo (ví dụ: pin yếu, nhiệt độ cao).
# Weaver++

Weaver++ là hệ thống xe đạp điện thông minh tích hợp BLE, RFID, CAN Bus và cảm biến, hướng tới bảo mật, giám sát hiệu năng và trải nghiệm người dùng tối ưu.

---

## Tính năng nổi bật
- **Bảo mật BLE**: Kết nối an toàn với mã hóa AES-128, bảo vệ MITM, quản lý thiết bị bonded tự động.
- **Xác thực kép**: Kết hợp nút BOOT và thẻ RFID.
- **Giao tiếp CAN**: Kết nối BMS và bộ điều khiển động cơ.
- **Giám sát cảm biến**: Thu thập tốc độ, nhiệt độ, trạng thái vận hành.
- **Giao diện trực quan**: Hiển thị thông tin xe trên màn hình LCD.

## Kiến trúc hệ thống
**Phần cứng**: ESP32, RFID, cảm biến tốc độ/nhiệt độ, CAN Bus, màn hình LCD.
**Phần mềm**: FreeRTOS đa nhiệm, NimBLE BLE, PlatformIO.

## Hoạt động hệ thống
### Main (logic trung tâm)
- Khởi tạo phần cứng, module BLE, RFID, BMS, VESC, cảm biến.
- Vòng lặp: cập nhật trạng thái, xử lý sự kiện, gửi dữ liệu tới display.

### Display (BikeDisplayUI)
- Nhận dữ liệu từ main qua CAN/BLE.
- Hiển thị tốc độ, pin, nhiệt độ, trạng thái kết nối, cảnh báo.
- Đổi màu UI theo ngưỡng cảnh báo, phản hồi trạng thái xe.

## Tài liệu chi tiết
- [docs/System_Overview.md](docs/System_Overview.md)
- [docs/BLE_Bonding_Limit.md](docs/BLE_Bonding_Limit.md)

- [Tổng Quan Hệ Thống](docs/System_Overview.md)
- [Giới Hạn Bonded Devices](docs/BLE_Bonding_Limit.md)

---

**Tác Giả**: SAOKIM99  
**Phiên Bản**: v2.0  
**Cập Nhật Lần Cuối**: 10/10/2025