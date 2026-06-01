# Clawdmeter-CYD

Cheap Yellow Display `ESP32-2432S028R`용 Clawdmeter fork입니다. Claude Code용
구성에 더해 Codex 사용량 표시도 지원합니다.

## 미리보기

![CYD UI preview](screenshots/cyd_2432s028r/preview.svg)

## 바로 설치

**웹 설치:** [https://sioaeko.github.io/clawdmeter-cyd/web-flasher/](https://sioaeko.github.io/clawdmeter-cyd/web-flasher/)

루트 주소도 바로 설치 페이지로 이동합니다:
[https://sioaeko.github.io/clawdmeter-cyd/](https://sioaeko.github.io/clawdmeter-cyd/)

Web Serial 지원 브라우저에서 CYD를 USB로 연결하고 **Install**을 누르면 됩니다.

## 이 fork에서 바뀐 것

이 저장소는 [HermannBjorgvin/Clawdmeter](https://github.com/HermannBjorgvin/Clawdmeter)를
Codex + CYD 보드용으로 개조한 fork입니다.

- CYD `ESP32-2432S028R` 보드 포트 추가
- PlatformIO env `cyd_2432s028r` 추가
- BLE 장치명 `clawdmeter`
- Codexmeter 전용 BLE UUID 사용
- 240x320 세로 화면용 compact UI
- 320x240 가로 화면용 firmware env와 웹 플래셔 옵션
- 펌웨어 안에서 `Used` / `Remaining` 표시 기준 전환 가능
- CYD 색감 보정을 위한 LCD inversion / red-blue swap 훅
- Codex 사용량 daemon 추가
- macOS LaunchAgent installer 추가
- GitHub Pages용 ESP Web Tools flasher 추가

## 사용 순서

### 1. 펌웨어 굽기

웹 플래셔를 엽니다.

[https://sioaeko.github.io/clawdmeter-cyd/web-flasher/](https://sioaeko.github.io/clawdmeter-cyd/web-flasher/)

CYD를 USB로 연결한 뒤 원하는 방향의 버튼을 누르고 USB Serial 포트를 선택하세요.
보드가 플래시 모드로 안 들어가면 `BOOT` 버튼을 누른 채 다시 시도하면 됩니다.

- **Portrait**: 240x320 세로 화면
- **Landscape**: 320x240 가로 화면

설치 후 `BOOT` 버튼을 누르면 `Used/Remaining` 화면, `Bluetooth` 화면,
`Settings` 화면 순서로 이동합니다. `Settings` 화면에서 표시 카드를 터치하면
사용량 기준 `Used`와 남은량 기준 `Remaining`이 전환되고, 설정은 보드 안에 저장됩니다.

### 2. Bluetooth 페어링

펌웨어 설치 후 보드를 켜고 컴퓨터에서 Bluetooth 장치를 찾습니다.

```text
clawdmeter
```

Windows와 Mac mini를 같이 쓴다면 **Mac mini에만 페어링**하는 구성이 가장 깔끔합니다.
Codex 사용량은 계정 기준이라 Windows에서 Codex를 써도 Mac mini daemon이 같은 계정으로
로그인되어 있으면 화면에 반영됩니다.

### 3. Mac mini에서 daemon 실행

```bash
codex login
./install-codex-mac.sh
```

로그 확인:

```bash
tail -F ~/Library/Logs/codex-usage-daemon.out.log
```

## Windows에서 쓰려면

Windows도 웹 플래셔로 펌웨어 설치는 됩니다. 다만 자동 daemon 설치는 현재 macOS 중심입니다.
수동 실행은 아래처럼 가능합니다.

```powershell
codex login
py -3 -m venv daemon\.venv
.\daemon\.venv\Scripts\python.exe -m pip install bleak httpx
.\daemon\.venv\Scripts\python.exe .\daemon\codex_usage_daemon.py
```

상시 표시용이면 Windows보다 Mac mini에 daemon을 켜두는 쪽을 추천합니다.

## 개발 명령

CYD 세로 펌웨어 빌드:

```bash
pio run -d firmware -e cyd_2432s028r
```

CYD 가로 펌웨어 빌드:

```bash
pio run -d firmware -e cyd_2432s028r_landscape
```

macOS에서 CLI 플래시:

```bash
./flash-mac.sh cyd_2432s028r
./flash-mac.sh cyd_2432s028r_landscape
```

펌웨어를 새로 빌드한 뒤 웹 플래셔 바이너리 갱신:

```bash
./tools/update_web_flasher.sh
```

CYD 프리뷰 이미지 재생성:

```bash
node tools/render_cyd_previews.js
```

웹 플래셔가 사용하는 factory images:

```text
web-flasher/firmware/clawdmeter-cyd_2432s028r.factory.bin
web-flasher/firmware/clawdmeter-cyd_2432s028r_landscape.factory.bin
```

## 파일 구조

- `firmware/src/boards/cyd_2432s028r/` - CYD 보드 포트
- `firmware/platformio.ini` - PlatformIO env `cyd_2432s028r`,
  `cyd_2432s028r_landscape`
- `daemon/codex_usage_daemon.py` - Codex 사용량 BLE daemon
- `install-codex-mac.sh` - macOS daemon 설치 스크립트
- `web-flasher/` - GitHub Pages 웹 플래셔
- `tools/update_web_flasher.sh` - 최신 factory binary를 웹 플래셔에 복사

## 하드웨어 메모

현재 펌웨어는 흔한 CYD 보드 핀맵을 기준으로 합니다.

- TFT: MISO 12, MOSI 13, SCLK 14, CS 15, DC 2, BL 21
- Touch: IRQ 36, MISO 39, MOSI 32, CLK 25, CS 33
- Button: BOOT/GPIO0 화면 전환
- Display setting: 보드 Settings 화면에서 `Used` / `Remaining` 전환

웹 플래셔에서 세로/가로 firmware를 고를 수 있습니다. 터치가 뒤집히거나 축이 바뀌면
아래 파일의 `TOUCH_*` 값을 조정하세요. 색이 반전되거나 빨강/파랑이 바뀌면 같은 파일의
`LCD_INVERT_COLORS` 또는 `LCD_SWAP_RED_BLUE` 값을 조정하면 됩니다.

```text
firmware/src/boards/cyd_2432s028r/board.h
```

## Upstream

원본 프로젝트는 Clawdmeter입니다. 이 fork는 Codex 사용량과 CYD
`ESP32-2432S028R` 보드에 맞춘 버전입니다.
