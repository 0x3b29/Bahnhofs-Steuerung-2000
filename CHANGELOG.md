## [1.5.0] - 2024-12-05
### Added
- Support for collapsable channel settings

## [1.5.0] - 2024-12-05
### Added
- Support for collapsable channel settings

## [1.4.0] - 2024-12-05
### Added
- Support for channel link delays

## [1.3.0] - 2024-12-05
### Added
- Support for lerping channels
- Support for delay between linked channels

### Changed
- Restructured channel settings

## [1.2.0] - 2024-07-15
### Added
- Support for servo motors
- Optional slider control for channels
- Support for custom PWM ranges
- Support for proportionally linked channels

### Fixed
- Resolve test range slider not updating correctly when range is changed

## [1.1.0] - 2024-02-20
### Added
- Ability to hide channels in compact view
- Increase usable limit of PWM boards to 16
- Overview of the memory layout

### Changed
- Use One based addresses for anchors as well

### Security
- Remove OTA update functionality to avoid random crashes
- switch to snprintf to avoid buffer overflows

## [1.0.0] - 2024-02-20
### Added
- Customizable channel names
- Customizable max brightness
- Customizable default state after power up
- Random on / off events
- Linkages among channels
- Persistent State via Eeprom
- Internationalization (I18n) for 5 languages

### Security
- Added CRC to Eeprom pages to detect bad states