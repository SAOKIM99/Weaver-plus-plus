# BLE Bonding Device Limit Management

## 📋 Tổng quan
Hệ thống BLE Bonding của dự án **Weaver++** đã được cải tiến để **giới hạn tối đa 5 thiết bị được bond** (lưu kết nối). Khi có thiết bị mới bond vượt quá giới hạn, thiết bị cũ nhất sẽ tự động bị xóa.

## 🎯 Yêu cầu
- **Maximum bonded devices**: 5 thiết bị
- **Auto-remove strategy**: FIFO (First In, First Out) - Xóa thiết bị cũ nhất

## 🔧 Implementation

### 1. Configuration (BLEBikeManager.h)
```cpp
#define MAX_BONDED_DEVICES 5  // Maximum number of bonded devices
```

### 2. Key Functions

#### `enforceMaxBondedDevices()`
- **Khi gọi**: Sau khi pairing/bonding thành công
- **Chức năng**: Kiểm tra số lượng bonded devices và xóa thiết bị cũ nếu vượt quá giới hạn
- **Location**: Được gọi trong `onAuthenticationComplete()`

```cpp
void BLEBikeManager::enforceMaxBondedDevices() {
    int bondedCount = getBondedDevicesFromStack();
    
    // Remove oldest devices until we're at or below the limit
    while (bondedCount > MAX_BONDED_DEVICES) {
        removeOldestBondedDevice();
        bondedCount = getBondedDevicesFromStack();
    }
}
```

#### `removeOldestBondedDevice()`
- **Chức năng**: Xóa thiết bị cũ nhất trong danh sách bonded devices
- **Cơ chế**: Sử dụng `ble_store_util_delete_peer()` API của NimBLE
- **Log**: In ra địa chỉ MAC của thiết bị bị xóa

```cpp
void BLEBikeManager::removeOldestBondedDevice() {
    ble_addr_t peer_id_addrs[MYNEWT_VAL(BLE_STORE_MAX_BONDS)];
    int num_peers;
    
    ble_store_util_bonded_peers(peer_id_addrs, &num_peers, MYNEWT_VAL(BLE_STORE_MAX_BONDS));
    
    if (num_peers > 0) {
        // Remove first device (oldest)
        ble_store_util_delete_peer(&peer_id_addrs[0]);
    }
}
```

### 3. Workflow

```
New Device Pairing
       ↓
onConnect() - Start security
       ↓
onConfirmPIN() - User confirms via BOOT/RFID
       ↓
onAuthenticationComplete() - Bonding success
       ↓
enforceMaxBondedDevices() - Check limit
       ↓
    [Count > 5?]
       ↙     ↘
     Yes      No
      ↓        ↓
Remove oldest  Continue
      ↓        ↓
   Repeat → Success
```

## 📊 Example Log Output

### Scenario: 6th device bonding (exceeds limit)
```
========================================
[BLE] ✓ PAIRING/BONDING SUCCESS
[BLE] - Connection: BONDED
[BLE] - Encryption: ENABLED
[BLE] - Peer Address: aa:bb:cc:dd:ee:ff
[BLE] Device saved - next time will auto-connect
[BLE] Current bonded devices: 6 / 5
[BLE] ⚠️  Bonded devices limit exceeded!
[BLE] Removing oldest bonded device...
[BLE] ✓ Removed oldest device: 11:22:33:44:55:66
[BLE] Remaining bonded devices: 5
========================================
```

## 🔒 Security Features

### Maintained Features
- ✅ **BLE 4.2+ Secure Connections**: AES-128 encryption
- ✅ **MITM Protection**: Man-in-the-middle attack prevention
- ✅ **PIN Confirmation**: Via BOOT button or RFID card
- ✅ **Random Private Address (RPA)**: Privacy protection
- ✅ **Auto-reconnect**: Bonded devices connect without pairing

### New Feature
- ✅ **Bonded Device Limit**: Maximum 5 devices with auto-cleanup

## 🧪 Testing Checklist

- [ ] Bond 5 devices successfully
- [ ] Verify all 5 devices can auto-reconnect
- [ ] Bond 6th device and verify oldest device is removed
- [ ] Check Serial log shows correct removal message
- [ ] Verify removed device requires re-pairing
- [ ] Test `printBondedDevices()` shows correct count
- [ ] Test `clearBondedDevices()` removes all devices

## 🛠️ API Reference

### Get Bonded Device Count
```cpp
uint8_t count = bleManager.getBondedDeviceCount();
```

### Print All Bonded Devices
```cpp
bleManager.printBondedDevices();
```

### Clear All Bonded Devices
```cpp
bleManager.clearBondedDevices();
```

## 📝 Notes

1. **NimBLE Storage Order**: Devices are typically stored in bonding order (FIFO)
2. **Persistence**: Bonding information survives ESP32 reboots
3. **Manual Clear**: Use `clearBondedDevices()` to reset bonding list
4. **Build Status**: ✅ Successfully compiled (654KB Flash, 38KB RAM)

## 🔄 Migration from Old System

| Old System | New System |
|------------|-----------|
| Manual MAC address storage | NimBLE Bonding stack |
| Preferences library | NimBLE ble_store API |
| No device limit | 5 device limit with auto-cleanup |
| No encryption | AES-128 encryption |
| Manual authentication | Auto-reconnect for bonded devices |

## 📅 Version History

- **v2.0** (Oct 9, 2025): Added bonded device limit (5 devices max)
- **v1.0** (Oct 9, 2025): Migrated to NimBLE Bonding system
- **v0.x**: Legacy MAC address storage system

---
**Project**: Weaver++ (+ SAO KIM +)  
**Author**: SAOKIM99  
**Last Updated**: October 9, 2025
