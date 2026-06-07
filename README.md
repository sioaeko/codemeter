# Clawdmeter-CYD

`ESP32-2432S028R` Cheap Yellow Display 보드에서 Claude Code 또는 Codex 사용량을 보여주는 Clawdmeter fork입니다.

![CYD UI preview](screenshots/cyd_2432s028r/preview.svg)

## 바로 설치

웹 플래셔:
[https://sioaeko.github.io/clawdmeter-cyd/web-flasher/](https://sioaeko.github.io/clawdmeter-cyd/web-flasher/)

루트 주소도 웹 플래셔로 이동합니다:
[https://sioaeko.github.io/clawdmeter-cyd/](https://sioaeko.github.io/clawdmeter-cyd/)

Chrome 또는 Edge에서 CYD를 USB로 연결하고 **Install**을 누르면 됩니다. Web Serial은 HTTPS 또는 `localhost`에서만 동작합니다.

## 핵심 기능

- CYD `ESP32-2432S028R` 보드용 펌웨어
- Portrait `240x320`, Landscape `320x240` 빌드
- Codex / Claude Code 사용량 BLE 표시
- 사용량 기준 `Used` 또는 남은량 기준 `Left` 표시 전환
- 보드 Settings 화면에서 표시 기준/테마/accent/Night Mode 설정
- Night Mode: 지정 시간 동안 화면 완전 꺼짐, 터치 시 임시 wake
- macOS LaunchAgent installer
- Windows 수동 daemon 실행 지원
- GitHub Pages용 ESP Web Tools flasher

## 펌웨어 설치

웹 플래셔에서 원하는 방향을 고릅니다.

- **Portrait**: 240x320 세로 화면
- **Landscape**: 320x240 가로 화면

CLI로 플래시하려면:

```bash
./flash-mac.sh cyd_2432s028r
./flash-mac.sh cyd_2432s028r_landscape
```

펌웨어를 새로 빌드한 뒤 웹 플래셔 바이너리를 갱신하려면:

```bash
./tools/update_web_flasher.sh
```

## 보드 Settings

이 펌웨어는 Wi-Fi/AP 웹 포털을 사용하지 않습니다. 설정은 보드의 `Settings` 화면에 저장됩니다.

Settings에서 바꿀 수 있는 것:

- 표시 기준: 사용한 비율 `Used` 또는 남은 비율 `Left`
- 화면 테마와 Claude/Codex accent
- Night Mode 켜기/끄기
- Night Mode 시작/종료 시간

Night Mode는 데스크톱 daemon이 Bluetooth payload에 같이 보내는 현지 시각을 기준으로 동작합니다. 부팅 직후 첫 사용량 payload를 받기 전에는 스케줄이 적용되지 않습니다. 기본값은 `23:00`부터 `07:00`까지이며, Night Mode 중 화면을 터치하면 기본 5분 동안 다시 켜집니다.

## 보드 조작

- `Usage` 화면: 사용량 표시
- `Bluetooth` 화면: BLE 상태 표시
- `Settings` 화면: 표시 기준/테마/accent/Night Mode 전환
- `Splash` 화면: Claude 애니메이션
- BOOT/GPIO0 버튼: 화면 전환
- Usage/Bluetooth 좌상단 아이콘 터치: Splash로 이동
- Splash 터치: Usage로 복귀

`Used`는 사용한 비율이고, `Left`는 남은 사용 가능 비율입니다. 설정은 ESP32 NVS에 저장되어 재부팅 후에도 유지됩니다.

## Bluetooth 페어링

컴퓨터에서 아래 BLE 장치를 찾습니다.

```text
Claude Controller
```

Windows와 Mac mini를 같이 쓰는 경우에는 보드를 **Mac mini에만 페어링**하고 Mac mini에서 daemon을 상시 실행하는 구성이 가장 안정적입니다. Codex 사용량은 계정 기준이라 Windows에서 Codex를 써도 Mac mini daemon이 같은 계정으로 로그인되어 있으면 보드에 반영됩니다.

Codex daemon과 Claude daemon을 동시에 같은 보드에 붙이면 Windows BLE에서 characteristic 검색이 흔들릴 수 있습니다. 한 번에 하나만 켜는 구성을 권장합니다.

## macOS daemon

Codex 사용량:

```bash
codex login
./install-codex-mac.sh
tail -F ~/Library/Logs/codex-usage-daemon.out.log
```

Claude Code 사용량:

```bash
./install-mac.sh
tail -F ~/Library/Logs/claude-usage-daemon.out.log
```

## Windows daemon

Codex:

```powershell
codex login
py -3 -m venv daemon\.venv
.\daemon\.venv\Scripts\python.exe -m pip install bleak httpx
.\daemon\.venv\Scripts\python.exe .\daemon\codex_usage_daemon.py
```

Claude Code:

```powershell
py -3 -m venv daemon\.venv
.\daemon\.venv\Scripts\python.exe -m pip install bleak httpx
.\daemon\.venv\Scripts\python.exe .\daemon\claude_usage_daemon.py
```

상시 표시용이면 Windows보다 Mac mini에서 daemon을 켜두는 쪽이 덜 번거롭습니다.

## 개발

CYD 세로 빌드:

```bash
pio run -d firmware -e cyd_2432s028r
```

CYD 가로 빌드:

```bash
pio run -d firmware -e cyd_2432s028r_landscape
```

로컬 웹 플래셔 테스트:

```bash
python3 -m http.server 8787 --directory web-flasher
```

열기:

```text
http://localhost:8787/
```

웹 플래셔 factory images:

```text
web-flasher/firmware/clawdmeter-cyd_2432s028r.factory.bin
web-flasher/firmware/clawdmeter-cyd_2432s028r_landscape.factory.bin
```

## 하드웨어 메모

기본 타깃은 흔한 CYD 보드 리비전입니다.

- TFT: MISO 12, MOSI 13, SCLK 14, CS 15, DC 2, BL 21
- Touch: IRQ 36, MISO 39, MOSI 32, CLK 25, CS 33
- Button: BOOT/GPIO0

터치가 뒤집히거나 축이 맞지 않으면 아래 파일의 `TOUCH_*` 값을 조정합니다. 색이 반전되거나 빨강/파랑이 바뀌면 같은 파일의 `LCD_INVERT_COLORS`, `LCD_SWAP_RED_BLUE` 값을 확인합니다.

```text
firmware/src/boards/cyd_2432s028r/board.h
```

## Upstream

원본 프로젝트는 [HermannBjorgvin/Clawdmeter](https://github.com/HermannBjorgvin/Clawdmeter)입니다. 이 fork는 Codex 사용량과 CYD `ESP32-2432S028R` 보드에 맞춘 버전입니다.
