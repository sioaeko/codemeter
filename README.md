# CodeMeter

`ESP32-2432S028R` Cheap Yellow Display, 일명 CYD에서 Codex 또는 Claude Code 사용량을 보여주는 BLE 사용량 미터입니다.

바로 설치:

[https://sioaeko.github.io/codemeter/web-flasher/](https://sioaeko.github.io/codemeter/web-flasher/)

Chrome 또는 Edge에서 CYD를 USB로 연결하고 **Install**을 누르면 됩니다. Web Serial은 HTTPS 또는 `localhost`에서만 동작합니다.

![CYD UI 미리보기](screenshots/cyd_2432s028r/preview.svg)

## 기능

- CYD `ESP32-2432S028R` 보드 지원
- 세로 `240x320`, 가로 `320x240` 펌웨어 제공
- Codex / Claude Code 사용량 BLE 표시
- `Used` 사용량 기준과 `Left` 남은량 기준 전환
- 보드 설정 화면에서 표시 기준, 테마, 강조색, Bluetooth, Night Mode 설정
- Night Mode: 지정 시간 동안 화면을 완전히 끄고, 터치하면 임시로 깨우기
- macOS Codex / Claude Code 데몬 설치 스크립트
- Windows 수동 데몬 실행 지원
- GitHub Pages 기반 ESP Web Tools 플래셔

## 배포 정리

이 빌드는 단순 사용량 UI와 설정 UI만 유지하고, 기존 repo에서 저작권/상표권 문제가 될 수 있던 제3자 마스코트, 브랜드 로고, 비공개 폰트 기반 자산을 모두 제거하거나 자체 구현으로 대체한 배포본입니다.

- 부팅 상태 화면은 자체 추상 미터 UI입니다.
- 펌웨어 폰트는 LVGL 내장 Montserrat를 사용합니다.
- BLE 장치명은 `CodeMeter`입니다.
- Claude Code, Codex, Anthropic, OpenAI는 호환 대상 설명에만 사용됩니다. 이 프로젝트는 해당 회사들과 공식 제휴가 없습니다.

## 플래시

웹 플래셔에서 원하는 화면 방향을 고르면 됩니다.

- **세로 화면**: 240x320
- **가로 화면**: 320x240

CLI로 플래시하려면:

```bash
./flash-mac.sh cyd_2432s028r
./flash-mac.sh cyd_2432s028r_landscape
```

펌웨어를 다시 빌드한 뒤 웹 플래셔 바이너리를 갱신하려면:

```bash
./tools/update_web_flasher.sh
```

## 보드 설정

Wi-Fi/AP 설정 포털은 사용하지 않습니다. 설정은 보드의 `Settings` 화면에 저장됩니다.

설정 화면은 두 페이지입니다. `NEXT` / `PREV`로 이동합니다.

1페이지:

- `Display`: `Used` 또는 `Left`
- `Theme`: `Dark` 또는 `Light`
- `Accent`: `Warm` 또는 `Green`
- `Bluetooth`: BLE 상태 화면 열기

2페이지:

- `Night`: Night Mode 켜기/끄기
- `Start`: 시작 시간을 1시간씩 이동
- `End`: 종료 시간을 1시간씩 이동

Night Mode는 데스크톱 데몬이 BLE payload에 같이 보내는 `now` 값을 기준으로 동작합니다. 부팅 직후 첫 payload를 받기 전에는 스케줄이 적용되지 않습니다.

## 블루투스

컴퓨터에서 아래 BLE 장치를 찾으면 됩니다.

```text
CodeMeter
```

Windows와 Mac mini를 같이 쓰는 경우에는 보드를 **Mac mini에만 페어링**하고 Mac mini에서 데몬을 상시 실행하는 구성이 가장 안정적입니다. 사용량은 계정 기준이라 Windows에서 Codex를 써도 Mac mini 데몬이 같은 계정으로 로그인되어 있으면 보드에 반영됩니다.

Codex 데몬과 Claude Code 데몬을 동시에 같은 보드에 붙이면 값이 서로 덮일 수 있습니다. 한 번에 하나만 켜는 구성을 권장합니다.

## macOS 데몬

Codex:

```bash
codex login
./install-codex-mac.sh
tail -F ~/Library/Logs/codex-usage-daemon.out.log
```

Claude Code:

```bash
./install-mac.sh
tail -F ~/Library/Logs/claude-usage-daemon.out.log
```

## Windows 데몬

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

## 개발

CYD 세로 빌드:

```bash
pio run -d firmware -e cyd_2432s028r
```

CYD 가로 빌드:

```bash
pio run -d firmware -e cyd_2432s028r_landscape
```

로컬 웹 플래셔:

```bash
python3 -m http.server 8787 --directory web-flasher
```

브라우저에서 열기:

```text
http://localhost:8787/
```

## 하드웨어 메모

기본 대상은 흔한 CYD 보드 리비전입니다.

- TFT: MISO 12, MOSI 13, SCLK 14, CS 15, DC 2, BL 21
- Touch: IRQ 36, MISO 39, MOSI 32, CLK 25, CS 33
- Button: BOOT/GPIO0

터치 방향이나 색이 뒤집혀 보이면 아래 파일을 확인하세요.

```text
firmware/src/boards/cyd_2432s028r/board.h
```

## 기반 프로젝트

이 저장소는 [HermannBjorgvin/Clawdmeter](https://github.com/HermannBjorgvin/Clawdmeter)의 CYD 포트에서 시작했습니다.

현재 CodeMeter 배포본은 사용량 화면과 설정 화면의 동작만 유지합니다. 기존에 포함되어 있던 저작권 민감 자산과 브랜드 전용 표현 요소는 모두 제거하거나 대체했습니다. 부팅/상태 UI는 자체 구현이고, 펌웨어 폰트는 LVGL 내장 Montserrat를 사용하며, 제3자 마스코트, 공식 로고, 비공개 폰트 자산은 이 저장소에 포함하지 않습니다.
