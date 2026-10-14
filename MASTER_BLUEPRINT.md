# tokkebi
## 통합 오디오 수집·샘플 라이브러리·DAW 플러그인: MASTER BLUEPRINT

> **문서 상태:** `ROADMAP RESTORED — CANONICAL 34-TASK PLAN` · canonical plan version `0.2.9-roadmap-recovery` · 작성 기준일 `2026-10-14`
> **대상 저장소:** https://github.com/nowyoullnever/tokkebi · 기본 브랜치 `main`
> **제품명:** `tokkebi`는 확정된 공식 제품명이다. 최종 앱 라이선스는 별도 결정이 필요하다.
> **현재 프로젝트 단계:** P02 Audio Editing 진행 중. Development Task 08은 COMPLETE / MERGED, Development Task 09는 다음 기능 작업으로 아직 구현되지 않았으며, Development Task 10은 PR #8의 early waveform work와 PR #9의 parked selection foundation으로 부분 구현 상태다. Development Task 11은 시작하지 않았다.
> **현재 권한:** Development Task 09는 다음 기능 작업이며, 별도의 명시적 소유자 작업 지시가 있어야 구현을 시작한다. Task 10의 parked work와 Task 11 이후의 새 기능 구현은 각각의 별도 승인 전까지 시작하지 않는다.
> **개발 기본 원칙:** 유료 API 의존성 없음 / Windows + macOS / Standalone + VST3 + AUv2 / 로컬 우선 / 불필요한 DRM 우회 없음 / 사용자의 샘플 데이터 보존.

---

# 00. 이 문서를 사용하는 규칙

### 00.1 단일 기준 문서(Single source of truth)

- `MASTER_BLUEPRINT.md`는 제품 목적, 범위, UI/UX, 디자인, 기술 구조, 저장 규격, installer, 출시·테스트 기준을 규정하는 **최상위 지침서**다. `AGENTS.md`는 코드 작성·리뷰 절차만 규정하며 이 문서의 기능 명세를 재정의할 수 없다.
- README, 이슈, PR, 코드 주석, 이전 Codex 프롬프트 사이에 충돌이 있다면 **사용자의 가장 최근 명시적 결정**을 우선하고, 이후 이 문서를 수정해야 한다. 변경이 반영되지 않았다면 확인 전까지 작업을 멈춘다.
- 이 문서의 모호함은 Codex가 취향으로 결정하지 않는다. `OPEN-...` 이슈로 보고해 사용자가 판단하도록 한다. 기능 삭제·축소를 임의로 정상이라고 선언하지 않는다.
- 문서를 조용히 다시 쓰지 않는다. 변경마다 버전, 날짜, 변경 이유, 연관된 요구사항 ID, 기존 데이터 호환성 영향을 기록한다.
- **현재는 검토용 초안이다.** 이 파일을 확정된 출시 계약으로 간주하지 말고 수정 후보를 자유롭게 제안한다. 승인 이후에는 기준선으로 사용한다.
- `필수/확장/조건부/제외/미확정`은 구현 여부를 뜻하며, `P00…P09`는 **구현 시점**을 뜻한다. 확장 기능도 제품 범위에는 포함되지만 1차 릴리스 보장은 아니다.

### 00.1.1 전역 Development Task 번호 규칙

Development Task 번호 `01–34`는 전역적이며 불변이다. `Pxx.N` 형식의 표기에서 `N`은 명시적인 correction suffix가 아닌 한 전역 Development Task 번호를 뜻한다. `P01.5 = Phase P01, Development Task 05`, `P02.8 = Phase P02, Development Task 08`, `P03.12 = Phase P03, Development Task 12`이다. `N`을 phase-local 임의 하위 단계로 재해석하지 않는다. `P02.8.1`처럼 추가된 `.1`은 새 전역 Task가 아니라 Task 08의 correction revision이다.

### 00.2 요구사항 상태와 증거

| 상태 | 정의 | 코드 검토 방식 |
|---|---|---|
| `MUST` | 최종 제품의 필수 기능 | 기능 테스트·검증 가능한 수락 기준 필요 |
| `SHOULD` | 권장 기능/개선 | 우선순위에 따라 단계 조절 |
| `FUTURE` | 확장 범위, 최초 출시 불필요 | 설계상 확장 가능해야 함 |
| `CONDITIONAL` | 호스트/서비스/권한별 지원 | 불가 시 대체 동작·명시적 안내 |
| `OUT` | 명시적으로 구현하지 않음 | UI에서 지원하는 듯 표현 금지 |
| `OPEN` | 결정·검증 미완료 | 소유자 승인 또는 기술 실험 전 확정 금지 |

각 요구사항의 ID는 영역 접두어 + 번호: `PRD-`, `VIS-`, `FNT-`, `UI-`, `WEB-`, `MATCH-`, `EDT-`, `JOB-`, `P2P-`, `SLSK-`, `INB-`, `LIB-`, `HIS-`, `DAT-`, `DAW-`, `ARC-`, `DEP-`, `INS-`, `SEC-`, `TST-`, `REL-`을 사용한다. 개별 PR은 어떤 ID를 구현했는지 명시해야 한다.

### 00.3 절대 불변 원칙

1. 외부 유료 API를 요구하지 않는다. 가입을 필요로 하는 서비스가 있다면 선택적으로만 연동한다.
2. 오디오 저장·검색·태그는 인터넷이 끊겨도 작동한다.
3. `History`에 오디오나 waveform, 썸네일·분석 데이터는 저장하지 않는다. **링크·태그·Description 텍스트만** 저장한다.
4. Inbox의 원본, 임시 캐시, Library에 저장한 최종 파일은 별개의 데이터다. **캐시 정리·히스토리 삭제·언인스톨로 최종 파일을 삭제하지 않는다.**
5. 외부 클라이언트(qBittorrent, SoulseekQt, slskd)의 사용자 데이터를 멋대로 삭제·이동하거나 시딩 설정을 변경하지 않는다.
6. 플러그인 오디오 콜백에서 네트워크 요청, 외부 실행 파일, DB·디스크 접근, 블로킹 대기 작업을 실행하지 않는다.
7. UI에서 **시스템 기본 폰트로 암묵적 폴백 금지**. 자체 번들 폰트만 사용한다(운영체제 관리 파일 선택 창 등 통제 불가능한 OS UI 제외).
8. DRM·접근 제어 우회 엔진은 제품에 포함하지 않는다. 사용·다운로드 권리가 있는 오디오에 초점을 둔다.
9. FL Studio·Ableton Live·Logic Pro 간 공통 워크플로를 제공하며, 호스트별 제약은 표시하고 대체 수단을 제공한다.
10. 설치·업데이트 실패 또는 Helper 오류가 DAW 프로젝트/라이브러리 파일을 손상시키면 안 된다.

# 01. 제품 목적·대상·경계

### 01.1 제품 정의

`PRD-001 [MUST]` 인터넷, 로컬 파일 및 사용자가 이미 설치한 P2P 클라이언트에서 찾은 소리를 **수집 → Inbox → Preview/Trim/Convert → Name/Tag/Description → 개인 Library → DAW Drag & Drop**으로 연결한다.

`PRD-002 [MUST]` Standalone이 독립적인 개인 오디오 라이브러리 역할을 하고 VST3/AUv2가 같은 데이터를 조회한다. 모든 환경에서 통일된 UI 원칙을 적용한다.

`PRD-003 [MUST]` 외부 유료 API, 계정, 클라우드 서버, AI 모델 실행·요금을 **필수 요건**으로 삼지 않는다. 사용하는 제3자 서비스의 정책과 네트워크는 별개의 종속성으로 고지한다.

`PRD-004 [MUST]` 강제적인 폴더 구조·태그 계층을 요구하지 않는다. 사용자는 임의의 텍스트로 분류할 수 있다.

`PRD-005 [MUST]` 사용자가 원본 URL, 실제 다운로드 URL, 자른 시작/끝 시간, 변환된 결과물을 구별할 수 있어야 한다.

`PRD-006 [OUT]` 음원 제공자 동의 없이 DRM을 해제하는 기능, 타사 스트리밍 보호를 뚫는 기능, 불법 콘텐츠를 검색하도록 사전 설정된 색인, 다운로드·샘플 저작권 자동 승인 기능은 범위 밖이다.

### 01.2 사용 시나리오 (E2E)

- **FLOW-A YouTube**: URL 붙여넣기 → 메타데이터 확인 → 필요한 구간 지정 → 다운로드/로컬 추출 → WAV → 이름·태그·메모 → Library → FL Playlist.
- **FLOW-B Spotify/Apple**: URL을 *곡 식별 힌트*로 해석 → 공개 메타데이터 확인 → YouTube 후보 검색 → 사용자가 오디오 출처를 확인·선택 → 정상 다운로드 가능한 콘텐츠를 처리 → 실제 출처 기록.
- **FLOW-C Bandcamp 앨범**: 목록 → 트랙별 선택 → 다운로드 큐 → Inbox → 여러 영역 별도 샘플 저장.
- **FLOW-D SoulseekQt**: 일반 SoulseekQt에서 파일을 받아 둠 → 완료 파일 감시 → Inbox → 일부를 Logic Quick Sampler로 이동.
- **FLOW-E slskd**: 플러그인에서 검색 → 공유 파일 확인·다운로드 → 완료 알림 → Editor/Library.
- **FLOW-F qBittorrent**: magnet/.torrent 입력 → 특정 오디오 파일 우선 다운로드 → 파일이 완성된 후 Inbox 편입 → 원본 Torrent 시딩 유지.
- **FLOW-G Revisit**: History에 남긴 URL+태그+Description 검색 → URL 재열기 → 같은 원본에서 다른 구간 샘플링.
- **FLOW-H Offline**: 인터넷 없음 → 저장된 샘플 검색/재생/수정 → DAW로 파일 전달, History 텍스트 열람.
- **FLOW-I Multi-project**: FL의 프로젝트 A, Ableton의 프로젝트 B가 서로 다른 프로젝트 폴더를 기억하며 같은 전역 Library를 공유.
- **FLOW-J Missing files**: 원본·외장 SSD 분리 → 오류/재연결 표시 → DB와 사용자 메모 보존.

### 01.3 플랫폼·제품 패키지

| 대상 | 1차 범위 | 참고 |
|---|---|---|
| Windows x64 | Standalone + VST3 | FL Studio, Ableton Live |
| macOS arm64 | Standalone + VST3 + AUv2 | FL Studio, Ableton Live, Logic Pro |
| macOS x86_64 | 동일 | Intel 빌드 / Universal 검토 |
| Linux | FUTURE | 현재 프레임워크·호스트 검증 별도 |
| CLAP | FUTURE | 지원 여부 후순위 |
| VST2 / AAX | OUT | 공식 초기 배포 범위 아님 |

`PRD-007 [MUST]` Plugin은 오디오 FX 슬롯에서 호스트를 손상시키지 않는 **투명 패스스루** 또는 필요한 형식의 유틸리티로 구현하고, Preview 신호만 사용자가 승인한 미리듣기 경로로 들려준다. Plugin bypass/unload/DAW render 동작을 테스트한다.

`PRD-008 [CONDITIONAL]` 직접 Playlist/Arrangement 삽입, 프로젝트 실제 파일 경로 조회는 DAW 공통 API가 보장하지 않으므로 기본 계약으로 약속하지 않는다. **OS 파일 Drag & Drop/독립 Helper 창/Show in Finder·Explorer/Copy Path**가 공통 기본 동작이다.

# 02. 브랜드·시각 디자인 시스템

### 02.1 콘셉트

`VIS-001 [MUST]` **Early Cyber / Y2K Futurism**. 1990년대 말~2000년대 초반 실험적 오디오 소프트웨어, 거친 픽셀화, 열화상 false color, posterization, 초기 CGI, 프랙탈 기하, 도트 매트릭스·CRT scanlines, noise·film grain·dithering을 사용한다. 고전 OS를 그대로 복각하기보다 실험적 그래픽과 실용적 조작을 결합한다.

`VIS-002 [MUST]` 현대식 SaaS 대시보드 모양(과도한 floating rounded cards, glassmorphism, pill 남용, 무의미한 거대 여백)을 피한다. 프레임, 마이크로텍스트, 압축된 데이터 창, 픽셀 아이콘, grid line, 라벨·계측 정보의 밀도감을 허용한다.

`VIS-003 [MUST]` **실용 UI 우선.** Grain·왜곡·scanlines·posterization은 배경과 장식 레이어에, waveform·시간 입력·텍스트·선택 경계에는 고대비 선명한 레이어를 사용한다. 사용자 입력 화면에 장식이 겹치지 않도록 한다.

### 02.2 고정 색상 팔레트

사용자가 제공한 12색 외의 브랜드 색을 무단으로 추가하지 않는다. 기능적 대비색·투명도·합성색은 Theme Token에서만 관리한다.

| 토큰 | HEX | 기본 의미 |
|---|---|---|
| `rose.600` | `#B06070` | 주 강조·선택 상태 |
| `blue.700` | `#0C54B4` | 액션·링크·선택 화살표 |
| `green.600` | `#189078` | 성공·Ready·Play |
| `brown.950` | `#403020` | 어두운 바탕·텍스트 |
| `rose.300` | `#F0A8B4` | 샘플 선택 영역 |
| `plum.800` | `#784860` | 세컨더리 레일/보조 패널 |
| `cyan.500` | `#50A0B0` | Waveform 기본색 |
| `ice.300` | `#A8CCE4` | 보조 패널·쿨 하이라이트 |
| `teal.900` | `#205050` | 어두운 오디오 캔버스 |
| `violet.500` | `#A080A0` | 메타데이터·보조 라벨 |
| `paper.100` | `#F0D8D8` | 밝은 표면·설명 영역 |
| `leaf.600` | `#489048` | 완료 후 저장 상태 |

`VIS-004 [MUST]` Light/Dark 두 모드를 지원하되 각 모드에서 실제 색상 대비 측정: 일반 텍스트 WCAG 4.5:1, 큰 텍스트 3:1 목표. 대비가 부족한 조합은 같은 팔레트 내 다른 배경/선·폰트 무게로 조정한다. 경고/오류 상태를 색상에만 의존하지 않고 아이콘·텍스트로 구별한다.

### 02.3 기하·모듈·치수

- 기본 레이아웃 grid 4px; 주요 간격 `4/8/12/16/24/32px`, 1px 혹은 2px 경계선, 두꺼운 구획은 3~4px.
- 기본 창 크기(조정 가능): **Standalone 1160×760**, **Plugin 1024×680**, 최소 **760×540**을 초기 기준으로 시제품 검증 후 변경한다. 고정 창이 아닌 responsive UI.
- 1150px 이상: 왼쪽 소스·리스트, 중앙 파형, 오른쪽 메타데이터의 3영역. 760~1149px: 2영역. 760 미만은 최소 제약 또는 compact overlay를 사용한다.
- 헤더 52px, 탭 레일 42px, 상태바 26px을 초기 기준으로 두되 DPI 스케일 조정. 터치 편의 클릭 타깃은 최소 28~32px 권장, 핵심 버튼 36px 이상.
- Button: 보통 직각/약한 베벨; `normal`, `hover`, `pressed`, `disabled`, `focused`, `busy`, `success`, `error` 상태 전부 구현.
- Progress: block segment/dot-matrix 기반. 막대의 의미(진행률 추정, 불명, 변환 단계)를 텍스트로 병행.
- Waveform canvas: 어두운 teal 바탕 위 밝은 cyan, 범위 rose tint, 스크러버 blue, selection handles rose. 2채널일 때 L/R 표시.
- 특유의 pixel/thermal 그래픽은 splash/background/header 장식에 제한하고 실제 waveform 해상도나 글리프 정밀도를 열화시키지 않는다.
- 아이콘: 1개의 통합 pixel icon set(프로젝트 자산으로 번들) 사용. 일반 이모지로 기능 버튼을 대체하지 않는다.
- 로고: 제품명 확정 전 텍스트 로고. 임시 로고도 폰트 규칙을 준수. 별도 브랜딩 파일로 교체 가능하게 설계.

### 02.4 폰트·라이선스 정책

`FNT-001 [MUST]` **운영체제 기본 폰트 사용 금지**. 앱이 렌더링하는 모든 텍스트는 저장소 또는 설치 패키지에 포함된 명시적 폰트 자산을 사용한다. 누락 문자 역시 승인된 번들 폰트로 폴백한다. 네이티브 OS 파일 대화상자 등 통제 불가능한 컴포넌트만 예외다.

| 폰트 | 기본 역할 | 확보 경로 / 상태 |
|---|---|---|
| **둥근모꼴+** | 제목·메뉴·버튼·주요 UI(우선순위 1) | Noonnu 공식 제공 정보 확인 후 라이선스 원문 보관 |
| **리디바탕** | 도움말, 설명, 긴 텍스트, 약관 | Noonnu 원본·허가 확인 |
| **Galmuri** **v2.40.4** | 서브 픽셀 폰트, 타임코드·표·숫자 | https://github.com/quiple/galmuri/releases/tag/v2.40.4 ; OFL 1.1 |
| **사용자 로컬 제4 폰트/추가 파일** | 파일 검사 후 역할 결정 | `C:\Users\Jung Chan\Desktop\font` — 현재 파일 미제공·미확인 |

- 사용자는 기본 폰트 네 종류를 원했으나 공개 대화로 확인 가능한 이름은 둥근모꼴·리디바탕·Galmuri 세 계열뿐이다. **네 번째 폰트가 확인되지 않았으므로 `OPEN-FONT-04` 상태로 둔다. 임의의 폰트를 제4 폰트로 선정하지 않는다.**
- **로컬 `font` 디렉터리 안의 실제 파일을 확인하고 배포 가능한 원본만 저장소 `assets/fonts/`에 업로드해야 한다.** 원본 폴더는 사용자 삭제 예정이므로 업로드 커밋과 CI 빌드·글리프 테스트가 끝나기 전 원본 삭제 금지.
- 앱 사용 폰트는 CDN·Windows 설치 폰트에 의존하지 않는다. 원본 재배포 조건, 저작권 표기, reserved font name, OFL/license text를 자산별 문서화한다. 둥근모꼴의 배포 허가 방식은 OFL이라고 임의로 표시하지 않는다.
- 예시 폰트 레벨(실제 글리프/배율 실험 후 확정): Header 20~28px 둥근모꼴, 탭 14~16px 둥근모꼴, 버튼 13~15px 둥근모꼴, 타임코드 12~14px Galmuri, 설명 13~15px 리디바탕. 고정 폰트 크기가 아닌 DPI 대응.
- License check/character coverage: 완성형 한글, 기호, Latin, URL, 프랑스어 악센트, 일본어 글자, 파일 경로, 특수기호 테스트. 합법적으로 재배포할 수 없는 자산은 포함하지 않는다.

### 02.5 모션·질감·오디오 피드백

`VIS-005 [SHOULD]` 화면 전환 100~180ms 이내, 8~16fps 추억의 애니메이션 스타일은 장식에만, Reduced Motion 사용 시 비활성화. 클릭 사운드 기본 OFF. 노이즈 효과 기본 강도 낮음, `OFF/LOW/NORMAL`, 읽기 어려우면 자동 OFF. 시스템 색상 모드와 무관하게 팔레트 유지 가능.

# 03. 정보 구조, 화면, 공통 UX

### 03.1 전역 화면 구조

6개 탭은 순서와 명칭 고정: **`WEB / P2P / INBOX / LIBRARY / HISTORY / SETTINGS`**. Standalone과 VST3/AU는 동일 컴포넌트·화면 상태를 공유한다. 현재 탭은 각 플러그인 인스턴스에서 기억한다.

```text
┏━━━━━━━━━━━━━━━━━━━━ tokkebi ━━━━━━━━━━━━━━━━━━━━ [?] [⚙] ┓
┃ WEB │ P2P │ INBOX │ LIBRARY │ HISTORY │ SETTINGS ┃
┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
┃ URL / ARTIST / TRACK / SEARCH                 [LOAD]     ┃
┃                                                           ┃
┃ [SOURCE LIST]  [ARTWORK / DETAILS] [METADATA & ACTIONS]   ┃
┃                ┌────────────────────────────┐             ┃
┃                │  L/R WAVEFORM / GRID       │             ┃
┃                │  IN [-----------] OUT      │             ┃
┃                └────────────────────────────┘             ┃
┃                [▶] [■] [↺]  IN 01:20.000 OUT 01:24.500  ┃
┃                                                           ┃
┃ Name [.................]  Tags [+]                        ┃
┃ Description [........................................]      ┃
┃ Format [WAV 24 / 44.1 / Stereo] Location [Global/Project] ┃
┃                         [SAVE SAMPLE] [DRAG TO DAW]       ┃
┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫
┃ HELPER: READY  │ JOBS: 02 │ LIBRARY: CONNECTED │ 1.0 GB ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
```

`UI-001 [MUST]` 공통 헤더: 이름/버전(Help 또는 About 접근), 탭, 현재 프로젝트 폴더, Helper/연결 상태. 하단 status bar: 현재 Job, 최근 결과/에러, 클릭 시 전체 큐 또는 진단 보기.

`UI-002 [MUST]` 모든 주요 입력·목록·버튼에 다음 상태가 있다: `idle/hover/focus/loading/empty/success/failure/disabled`. Empty state는 구체적인 다음 행동을 말해야 한다. Tooltip은 텍스트와 단축키를 함께 제공한다.

`UI-003 [MUST]` 창 크기 조정, 레일 collapse, 리스트 resize, 스크롤·키보드 탐색, 짧은/긴 메모 wrap, 경로 말줄임(hover 전체 보기), 대량 목록 가상화, 비동기 작업 표시, 클릭 판정 일관성을 구현한다.

`UI-004 [MUST]` 작업 중 탭 이동·창 닫기: 현재 다운로드는 Helper에서 계속; 저장하지 않은 Name/Tag/Description/IN/OUT은 확인 또는 draft 자동 보존; 사용자 의도와 무관하게 데이터 버리기 금지. 같은 파일을 여러 창에서 편집하면 충돌 감지.

`UI-005 [MUST]` 외부 링크는 사용자의 기본 브라우저로 연다. 앱에 명령을 내리는 링크 실행 금지. 로그 파일·저장 위치 열기는 명시된 버튼으로만 한다.

### 03.2 공통 조작

- `Ctrl/Cmd+V`: 포커스한 필드에 붙여넣기; 통합 입력창 포커스 상태에서 URL 붙여넣기. URL 자동 조회는 기본 OFF(Enter/LOAD로 시작).
- `Enter`: 입력/검색 실행. 메모 입력 필드에서는 줄바꿈 또는 확정 동작과 충돌하지 않음.
- `Esc`: 모달 닫기, 마커 드래그 취소, 현재 입력 Esc 동작. **진행 중 다운로드 전체를 무조건 취소하지 않는다.**
- `Space`: Editor 포커스에서 Preview Toggle. DAW 키 입력이 우선일 수 있으므로 Shortcut capture 옵션; 텍스트 입력 중에는 스페이스 입력.
- `I/O`: 각각 IN/OUT 마커. `L`: Loop, `+/-`: 파형 확대/축소, `Ctrl/Cmd+S`: sample 저장. `Ctrl/Cmd+Z/Shift+Z`: Editor Undo/Redo. `F2`: 파일명 수정. `Ctrl/Cmd+F`: 현재 탭 검색.
- 메뉴·버튼은 키보드 포커스·Tab 및 Shift+Tab 탐색 지원. 주요 목록 다중 선택 `Ctrl/Cmd`, 연속 선택 `Shift`. 키보드 IME 조합 중 단축키 가로채지 않음.
- 마우스: 파형 클릭 재생 위치, 드래그 선택, 핸들 이동, modifier+휠 확대, 패널 세로 스크롤, 더블클릭 로드, 오른쪽 클릭 상황별 메뉴, 빈 공간 클릭 선택 해제.

### 03.3 메뉴별 최소 동작 표준

**샘플:** `Preview`, `Open Editor`, `Rename`, `Edit Tags`, `Edit Description`, `Copy Path`, `Copy Source URL`, `Open Source`, `Reveal in Explorer/Finder`, `Export`, `Remove from Library`, `Delete File…`.

**History:** `Load Source`, `Open Source`, `Copy URL`, `Edit Tags`, `Edit Description`, `Delete Entry`.

**검색 결과:** `Select Candidate`, `Open Source`, `Copy URL`, `Add to Queue`, `Search Again`.

**다운로드 작업:** `Details`, `Retry`, `Cancel`, `Remove Completed`, `Reveal File`, `Show Log`.

### 03.4 모달·알림 규칙

- Warning은 데이터 삭제·외부 상태 변경·덮어쓰기를 요구하는 경우에만 blocking 모달로 제공.
- 일반 성공·오류는 취소 가능한 non-blocking 알림 + status bar. 오류는 `원인/대상/다음 행동`이 포함된 텍스트로 기록.
- Confirm은 기본 취소가 안전한 선택. `Delete from list`와 `Delete actual audio`를 명확하게 구분.
- 비동기 작업은 진행 상태가 표시되고, 취소 가능한 작업만 Cancel 버튼 활성화. 닫아도 데이터가 남는 경고를 명확히 표시.
- 시스템 알림은 사용자가 켠 경우에만 사용한다. 앱 내부 알림은 각 이벤트를 중복 표시하지 않는다.

### 03.5 접근성·국제화

`UI-006 [MUST]` 한국어 UI를 기본으로 하고 영어 전환. UI 문자열은 중앙 리소스 테이블에서 관리하며, 동일 오류의 표현을 통일. 한국어 조사·날짜·숫자/초 표시와 소수점 지역화, CJK·Unicode·긴 URL 지원.

`UI-007 [MUST]` 키보드-only 핵심 조작, 포커스 상태, 고대비 모드, 감소된 애니메이션, 상태를 색 외 텍스트로 표시, 레이블·툴팁 제공, 터치패드 가로/세로 스크롤 지원. OS 스크린리더 접근 가능 여부는 사용한 GUI 기술에서 지원하는 API 범위를 검증해 문서화.

# 04. WEB 탭: 소스 입력·인식·수집

### 04.1 입력의 종류

`WEB-001 [MUST]` 하나의 Input에서 다음을 식별한다.

| 입력 | 처리 |
|---|---|
| YouTube/YT Music 영상 URL | yt-dlp 메타데이터 조회 |
| Bandcamp 트랙·앨범 | 지원하는 트랙/리스트 해석 |
| SoundCloud 트랙·세트 | 지원 extractors 해석 |
| Vimeo·TikTok·Instagram 등 | yt-dlp 지원 범위에서 시도; 사이트 접근·정책에 따라 불가 가능 |
| 일반 yt-dlp 호환 URL | 안전 검증 후 resolver |
| 직접 HTTP(S) 오디오 URL | 파일 타입/길이/다운로드 확인 |
| Spotify 곡 URL | 메타데이터만 해석 후 대체 소스 검색 |
| Apple Music 곡 URL | 메타데이터만 해석 후 대체 소스 검색 |
| magnet: URL | P2P/qBittorrent로 전환 |
| .torrent 파일 | P2P/qBittorrent로 전환 |
| 로컬 파일/오디오 폴더 | Inbox 가져오기 |
| 일반 문자열 | Music/YouTube 검색 |

`WEB-002 [MUST]` URL trim·whitespace 제거·단축 URL follow·불필요한 추적 query 제거는 **원본 URL 보존과 별개**로 한다. 입력 문자열, 표시용 canonical URL, 실제 전송용 URL은 구분한다. 단축 링크가 열리지 않으면 원본을 보존하고 실패 이유 표기.

`WEB-003 [MUST]` 클립보드 붙여넣기, URL Drag & Drop, 여러 줄 URL 추가, 중복 입력 감지, 검색 중 취소, 새 입력 시 이전 비동기 응답이 새로운 화면을 덮어쓰지 않는 Request ID·cancel token을 구현한다. 클립보드 자동 감시는 옵트인.

### 04.2 조회 단계

`WEB-004 [MUST]` 조회 시 가능한 메타데이터: `source`, `title`, `uploader/artist`, `album`, `duration`, `source ID`, `thumbnail`, `release/upload date`, `track no`, `formats/codec/bitrate`, `estimated size`, `access state`, `source URL`. 누락 값은 `Unknown` 또는 빈 칸으로 표시; 유추 정보를 사실로 표시하지 않음.

`WEB-005 [MUST]` 메타데이터 조회와 실제 오디오 다운로드를 분리한다. `LOAD`는 파일 저장 명령이 아니다. Preview를 요청하면 필요한 임시 proxy/stream을 가져와야 할 수 있으므로 네트워크 전송 사실과 진행률 표시.

`WEB-006 [MUST]` 소스 뷰는 Art/Title/Artist/Album/Length/Platform/Source ↗/가능한 Format/Download permitted status/Cache status를 표시한다. 동영상 화질 선택 기능은 기본 범위 밖이며 오디오 트랙이 중심이다.

### 04.3 섹션 다운로드

`WEB-007 [MUST]` 전체와 부분 구간 다운로드: `HH:MM:SS.mmm` IN/OUT 입력, 파형/마커로 지정, 작은 구간 허용(최종 정밀도·최소 단위 검증), 여러 구간(후속), 부분 다운로드 불가 시 로컬 fallback. `yt-dlp --download-sections`와 ffmpeg 사용을 기본 검토한다.

`WEB-008 [MUST]` 네트워크 측 **실제 전송량 감소를 보장할 수 없음**. 스트림·코덱·서버 범위 요청에 따라 전체를 받아 로컬에서 잘라야 할 수 있다. 이 경우 사용자에게 고지하며 다운로드 단계 및 캐시 사용량을 표시한다. 실제 출력 시점 정밀도는 디코딩 이후 sample-accurate 검증을 목표로 한다.

`WEB-009 [MUST]` 이미 임시 원본이 다운로드되어 있으면 두 번째 구간 추출 시 해당 cache를 재사용한다. 원본 다운로드가 필요한 처리와 샘플 저장은 서로 분리한다. 임시 캐시 삭제는 Library에 저장된 샘플에 영향을 주지 않는다.

### 04.4 리스트·배치

`WEB-010 [SHOULD]` YouTube Playlist, Bandcamp Album, SoundCloud Set 등 **추출기가 제공할 수 있는 목록**을 lazy-load, 제목/길이/트랙 번호, 체크박스, 전체/부분 선택, 순서 보존, 공통 태그·Description, 중복 건너뛰기, 배치 전 용량 추정과 실패 항목 재시도를 지원한다.

`WEB-011 [MUST]` `Search/Resolve/Preview/Download/Convert` 각각에 독립적인 로딩·오류 상태가 있다. 인증 제한, 403/404/429/5xx, extractor 변경, 유료/DRM/지역 제한을 구분해서 안내한다. 소스를 지원하지 않을 경우 임의 대체 링크나 불법 우회 경로를 자동 실행하지 않는다.

### 04.5 검색 결과 및 재조회

`WEB-012 [MUST]` 검색어 입력 → 결과 카드(제목/채널/길이/썸네일/오디오 가능 여부) → `Open Source/Select/Refine Query`. 결과에 제목·아티스트·길이가 충분하지 않으면 `Manual entry`가 가능. YouTube 검색 결과는 권리가 자동 부여된 소스가 아니다.

# 05. Spotify / Apple Music 곡 식별과 YouTube 후보 매칭

### 05.1 지원 범위·경계

`MATCH-001 [MUST]` Spotify·Apple Music 링크는 직접 DRM 스트리밍 다운로드 URL이 아니다. **곡을 식별하는 메타데이터 소스**로만 사용한다. 실제 오디오 취득에 사용된 YouTube 등 별도 출처는 provenance에서 명확히 구분한다.

`MATCH-002 [CONDITIONAL]` Spotify oEmbed(제목·썸네일 등 공개 정보) → 수동 조정/가능한 경우 MusicBrainz 보강. Spotify OAuth·Premium·Web API를 기본 필수로 만들지 않는다. 정책 검토 결과 위험한 cross-service 매칭·표시를 비활성화할 수 있도록 feature flag로 독립 모듈화한다.

`MATCH-003 [CONDITIONAL]` Apple Music 주소에서 track ID와 store region을 해석하고 iTunes Search/Lookup API의 공개 메타데이터를 사용한다. `artistName/trackName/collectionName/trackTimeMillis/trackNumber/artworkUrl` 중 실제 제공 필드만 기록한다. 지역별 ID 및 응답 미일치를 검사한다. API 속도 제한·캐시 적용.

`MATCH-004 [SHOULD]` MusicBrainz는 제목/아티스트/앨범/녹음 길이/ISRC 보강에 한정하고 평균 초당 1요청 이하 제한 및 고유 User-Agent를 지킨다. 외부 API 결과는 절대로 이용 허가 또는 동일 녹음 증거로 간주하지 않는다.

### 05.2 검색 로직

`MATCH-005 [MUST]` YouTube `ytsearch`/음악 검색 주소 등 무료 조회 가능 수단을 사용하고, 쿼리 후보를 정규화하여 `Artist + Title`, 앨범 포함, 특수 문구 제거 순으로 시도한다. 첫 결과 자동 다운로드 금지; 결과 5~10개(설정값) 비교·표시. 검색 구조 변경/요청 제한 시 수동 URL 모드 제공.

`MATCH-006 [MUST]` 점수 후보(예시): 제목 +30, 아티스트 +25, 길이 ±2초 +20, 공식 Artist/Topic +15, 릴리스 버전 +10; Live/Cover/Remix/Slowed/Sped-up/Karaoke/Reaction/8D 등의 원곡과 불일치 감점, 길이 차이 감점. 점수는 **검색 유사도**이지 동일성 확률이나 라이선스 인증이 아니다.

`MATCH-007 [MUST]` Unicode normalization, 공백·기호, 괄호, 한글/영문 표기, feat/ft, 앨범판·싱글판, Remastered, clean/explicit, official music video intro, duplicate uploads, 동일 제목 다른 아티스트를 고려한다. 타이틀 동률이면 사용자 선택을 요구하고, 낮은 점수에서는 추천을 자동 확정하지 않는다.

`MATCH-008 [MUST]` 후보 카드: 출처/제목/채널/길이/일치 점수와 이유(예: 길이 3초 차이)/원본 열기/Select/검색어 수정/재검색. 사용자는 바로 URL을 수동으로 넣을 수 있다.

`MATCH-009 [SHOULD]` Album/Playlist 인식은 공개 API만으로 모든 트랙을 가져올 수 없을 때 가능한 부분만 제공한다. 전체 목록·정확한 길이를 모르는 상황에서 아는 척하지 않는다. Spotify/Apple Music의 공식 정책에 위반할 위험이 확인되면 기능 범위를 축소해야 하며, 본 문서의 `OPEN-LEGAL-01` 항목에 기록한다.

# 06. 오디오 Editor / Waveform / Preview / Export

### 06.1 Editor 진입·상태

`EDT-020 [MUST, P02.8 amendment]` 로컬 파일 기반의 최소 기반 단계에서는 파일 내용을 우선 검사하고 WAV PCM 16/24/32-bit 및 IEEE float 32-bit만 interleaved float PCM으로 decode한다. AIFF/AIFC와 FLAC은 컨테이너를 식별하되, 새 외부 decoder dependency를 추가하지 않고 `UnsupportedFormat`으로 명시한다. 상태는 인스턴스별 `Empty/Loading/Ready/Failed`이고, 파일·디렉터리·손상·절단·잘못된 메타데이터·용량 초과는 구조화된 오류로 표시한다. 이 amendment는 waveform, preview, 선택, 변환·export, persistence를 승인하지 않는다.

`EDT-001 [MUST]` WEB(원격 소스), Inbox(완료 파일), Library(저장된 샘플), P2P(완성 파일) 어디서든 동일 Editor 화면으로 연다. 원본을 열었는지 이미 잘린 파일을 열었는지 명확히 표시. 하나의 Editor 문맥에는 `sourceReference/localFile/selection/previewState/formatOptions/draftMetadata`를 기록한다.

`EDT-002 [MUST]` 메타데이터만 조회한 상태에서 즉시 완전한 Waveform을 가정하지 않는다. `Metadata Only`(제목 등), `Fetch Preview`(필요한 오디오 임시 확보), `Section First`(숫자 시간으로 우선 구간 다운로드), `Cached Full`을 UI에 구분한다. 네트워크 소스의 즉시 scrub 가능 여부는 실제 스트림/캐시에 따른다.

### 06.2 Waveform 세부

`EDT-003 [MUST]` 파일 decode 후 피크 캐시를 만들고 L/R 채널 개별 보기, mono 표시, 시간 눈금, 재생헤드, IN/OUT, 선택 영역 하이라이트, 파일 길이, 확대 레벨, 가로 스크롤, 빠른 재렌더링, 대형 파일 다중 해상도 peak level을 지원한다. 화면 표시 데이터와 오디오 실제 sample index가 혼동되지 않도록 한다.

`EDT-004 [MUST]` 파형 상 클릭으로 위치 이동, 드래그로 선택 범위, 마커 핸들 드래그, Shift/Alt 등 modifier 미세 조정, Wheel 줌, `Zoom to selection/Zoom to full`, 빈 곳 클릭 해제, marker의 위치 수치 입력, 선택 길이 즉시 표시.

`EDT-005 [MUST]` 분 단위 이상의 긴 자료도 무리 없이 표시(부분 cache, 축소 피크), 긴 파일 자동 분석 중에도 기본 UI 응답 유지. decode 불가능한 파일은 waveform을 임의 생성하지 않고 원인과 필요한 조치를 표시한다.

`EDT-006 [FUTURE]` RMS/Peak 선택, spectrogram, transient markers, beat grid, zero-crossing 표시, bar/beats time ruler, 주파수 구간 미리보기, 채널별 확대.

### 06.3 미리듣기 조작

`EDT-007 [MUST]` `Play/Pause/Stop/Return to start/Seek/Selection Play/Loop/Whole Loop/Preview Gain/Mute/Time elapsed` 지원. 정지 시 playhead 복원 방식은 설정에서 선택; Loop on/off 상태가 시각적으로 구별된다.

`EDT-008 [MUST]` 재생 중 Slider/Marker 이동, 파일 전환, 탭 전환, Helper 재연결, DAW 호스트 transport 변화 때 안전한 동작. Preview 볼륨은 출력 파일의 원본 gain과 독립이며, 의도치 않은 clipping을 방지하는 감쇄·미터 제공.

`EDT-009 [CONDITIONAL]` Plugin에서는 호스트가 제공하는 오디오 I/O를 이용하고 유틸리티 패스스루 정책을 유지한다. Standalone에서는 CoreAudio/ASIO/WASAPI 등 가능한 실제 장치 설정을 제공한다. 호스트마다 오디오 출력이 제한되면 독립 Helper 미리듣기 창·Standalone 연계 등 대안을 보여준다.

### 06.4 선택 영역과 시간

`EDT-010 [MUST]` IN, OUT를 밀리초 단위로 표시·입력한다. 내부 계산은 가능한 경우 sample frame index로 변환해 출력 경계를 검증한다. `IN>=0`, `OUT>IN`, `OUT<=valid duration`, NaN/무한대/시간 파싱 오류 처리. 긴 영상과 24시간 초과 파일에도 범위를 정확히 검사한다.

`EDT-011 [MUST]` 선택 Undo/Redo, 전체 선택, 지우기, 숫자 입력, 선택 영역 이동, 선택 길이 표시, 해당 구간 반복, 시작·끝 주변 zoom. 파일 일부만 로드됐을 경우 가용/미가용 구간 시각적으로 구분.

`EDT-012 [FUTURE]` 다중 구간 선택, 영역별 이름, 영역 복제/삭제/정렬, batch export, zero-crossing snap, transient snap, beat snap, marker 목록, 영역을 연속 붙이는 concatenate export.

### 06.5 오디오 변환·처리

`EDT-013 [MUST]` 기본 출력: **WAV, AIFF, FLAC**. 채널: 원본 / Mono / Stereo; 샘플레이트: 원본 / 44.1 / 48 / 88.2 / 96 kHz, 비트: 16-bit PCM / 24-bit PCM / 32-bit float (파일 포맷에서 가능한 조합만). 인코더 옵션 불일치 시 안내.

`EDT-014 [MUST]` 선택 가능: Trim, Peak Normalize, 짧은 Fade In/Out, DC Offset Remove, Trim Silence. 모든 processing 옵션은 기본적으로 원본 변경 없이 별도 export에만 적용. 작업 전/후 preview는 render 가능 시 비교.

`EDT-015 [MUST]` 현재 소스의 실제 코덱/비트레이트/채널/샘플레이트 표시. 압축 손실 음원을 WAV로 바꾼다고 음질이 향상된다는 표현 금지. 인코더가 원본에서 생성한 결과의 clipping/length/format 검증.

`EDT-016 [MUST]` 오디오 변환은 백그라운드 Helper에서 실행; 각 Job은 Cancel/Progress/Result/Failure를 갖는다. 임시 파일에 완성한 뒤 파일 시스템 수준의 atomic rename 또는 동등한 crash-safe commit 수행; 생성 실패 시 최종 이름으로 깨진 파일을 저장하지 않는다.

`EDT-017 [FUTURE]` BPM/key detection, time stretch, pitch shift, reverse, loudness normalize(LUFS), auto-slice, loop marker / crossfade, speed preview, BPM/key 기반 파일명, 고급 오디오 특징 분석. 정확한 결과를 보장할 수 없는 자동 인식은 사용자 편집 가능하게 한다.

### 06.6 Export / Save Sample 버튼

`EDT-018 [MUST]` Name, Tags, Description, 원본·실제 출처, 구간, 출력 형식, 저장 경로를 확인하고 `SAVE SAMPLE`. 저장 중 중복 파일명이 있다면 `Rename / Create Version / Replace (confirm) / Cancel` 선택. Save 실패 시 사용자가 작성한 metadata draft 유지.

`EDT-019 [MUST]` `Save and Drag`를 누르면 **최종 파일이 완성될 때까지 외부 Drag를 시작하지 않는다**. 운영체제/호스트가 drop을 받지 못하면 file reveal/copy path/Floating Drag Window를 안내.

# 07. Download / Task Manager

### 07.1 공통 상태 기계

`JOB-001 [MUST]` 모든 source adapter는 다음 상태를 공통 이벤트로 표시한다:

`Created → Resolving → (Searching/Waiting) → Queued → Downloading → Verifying → Processing → Importing → Completed`.

대체 종료 상태: `Paused`, `CancelRequested`, `Cancelled`, `RetryWaiting`, `Failed`, `Unsupported`, `BlockedByPolicy`. 실제 source가 Pause를 지원하지 않으면 **중지/재시도**로 명시하고 가짜 pause 기능을 제공하지 않는다.

`JOB-002 [MUST]` Task에는 최소 `jobId`, `sourceId`, `stage`, `progress {known? current,total,unit}`, `speed`, `ETA if known`, `errorCode`, `errorMessage`, `createdContext`, `outputFiles`, `retryable`, `ownerInstance` 등을 관리한다. `History` 규칙과 달리 Task DB에는 실행 안정성에 필요한 운영 기록을 저장할 수 있으나 user history UI와 혼동하지 않는다.

### 07.2 Queue UI·동작

`JOB-003 [MUST]` 큐 등록, 제목·출처·단계·속도·퍼센트·완료/실패, 상세 로그, 실패 재시도, 취소, 큐 순서/동시성 제한, 모든 완료된 항목만 목록에서 지우기, 앱 종료 후 상태 복구. 알 수 없는 전체 크기/ETA는 정직하게 `—` 또는 indeterminate 표시.

`JOB-004 [MUST]` 사이트/adapter별 동시 작업 제한, 네트워크 및 디스크 용량 오류, 중복 URL·동일 트랙 중복 경고, idempotent retry, 실패 파일 재활용 및 정리. 다운로드 단계와 변환 단계를 구분해 진행률이 되돌아간 것처럼 보이지 않도록 단계 표시.

`JOB-005 [SHOULD]` 우선순위 이동, 전체 일시정지, 속도 제한, 연결 상태에 따른 자동 보류, 오류 알림, 상세 로그 복사, 같은 URL 캐시 사용, Batch 작업에서 실패 항목만 재시작.

### 07.3 파일 생명주기

`JOB-006 [MUST]` `tmp → verified original/cache → Inbox ready → exported sample` 파일 상태를 명시. `.part`/미완성 파일은 일반 샘플로 열 수 없으며, 다운로드 완료 이벤트만으로 진실로 간주하지 않고 파일이 안정화/검증되었는지 재확인한다. 파일 끝 길이·해시·코덱 확인을 지원한다.

`JOB-007 [MUST]` 모든 실행 로그에 URL의 민감한 토큰, 인증 쿠키, 개인 경로의 민감 부분을 마스킹. 외부 사이트에서 넘어온 출력 로그를 명령/URL로 해석하여 실행하지 않는다.

# 08. P2P: qBittorrent

### 08.1 외부 클라이언트 연동 원칙

`P2P-001 [MUST]` 프로그램 자체에서 BitTorrent 엔진/트래커를 운영하지 않고 **기존 qBittorrent WebUI API**를 통해 사용자가 권한을 가진 Torrent를 추가·조회한다. qBittorrent 설치 여부와 WebUI 활성 상태는 별도. 없으면 메뉴에 `Not connected`와 설정 가이드를 표시할 뿐 WEB/LIBRARY는 정상 작동해야 한다.

`P2P-002 [MUST]` 연결 마법사: 자동 추정 가능한 로컬 WebUI 엔드포인트 검사(공격적 포트 스캔 금지), 수동 `host/port/username/auth`, test, API version, errors(꺼짐/인증/403/timeout), disconnect. 자격 증명은 Keychain/Windows Credential Manager 등 안전 저장소 이용. 기본 바인딩 localhost, 원격 접속은 FUTURE·옵트인.

### 08.2 Magnet 및 파일 목록

`P2P-003 [MUST]` `magnet:` URI·`.torrent` 파일 drop·수동 지정. 기존 Torrent와 정보 해시가 같으면 추가 전에 경고. Torrent metadata 수신 중에는 파일 리스트 대기 상태 명확히 표시.

`P2P-004 [MUST]` Torrent 이름, 총 크기, 전송 상태, seed/peer, 예상 속도, 남은 시간, 저장 위치, hash, 파일 트리, 폴더 expand, 파일별 크기·완료 진행률·우선순위·오디오 필터를 표시한다. 파일 목록 조회 후 오디오 확장자/실제 디코딩 가능 여부를 구별한다.

`P2P-005 [MUST]` 파일 선택 → qBittorrent 파일 우선순위 조정(해당 Torrent에 한함), 다운로드 시작, 진행률 polling, 실제 완료된 파일만 Inbox에 등록. 타인이/사용자가 이미 추가한 Torrent의 우선순위를 무단 변경하지 않고 새로 만든 작업에 한정하거나 사용자 확인을 요구한다.

`P2P-006 [MUST]` **파일 복사/가져오기와 Torrent 시딩·원본 다운로드를 독립 처리**. 파일 읽기 이후 qBittorrent의 원본/다운로드 경로/ratio/seeding/트래커를 변경하지 않는다. 샘플 저장 때문에 torrent 원본을 삭제하거나 자동 정지하지 않는다.

`P2P-007 [SHOULD]` 외부 qBittorrent 검색 플러그인을 사용자가 이미 활성화한 경우 검색 가능 여부를 확인(기본 내장 불법 콘텐츠 인덱스 없음), 완료 Torrent에서 선택 파일 재가져오기, qBittorrent category/tag 분리, 원본 경로 변경 감지.

### 08.3 오류 처리

설치 없음, 실행 안 됨, WebUI 비활성, 인증 불일치, API 버전 변경, magnet malformed, metadata 없음, seed 없음, unavailable, 디스크 꽉 참, 다운로드 폴더 제거, partial download, piece verification 실패, qBittorrent 강제 종료, 기존 Torrent 겹침을 구분한다. 정확한 근거 없을 때 `파일 완료` 표시 금지.

# 09. P2P: SoulseekQt / slskd

### 09.1 두 가지 호환 모드

`SLSK-001 [MUST]` **SoulseekQt Limited Mode**: 공식 통제 API를 전제로 하지 않는다. 사용자가 지정한 완료 다운로드 폴더에 대해 watcher를 사용한다. 신규 파일·기존 폴더 스캔, 하위 폴더, 오디오 필터, 변경 감지, 파일 크기/mtime 안정화, 사용 중 잠금 확인 후 Inbox로 등록.

`SLSK-002 [MUST]` 감시는 자동 활성화 전에 폴더 선택과 사용자 동의를 요구한다. 해당 폴더 원본 파일 삭제/변경 금지; watcher가 활성화되어 있어도 다운로드 요청이나 remote search는 **SoulseekQt에서 수행**한다. GUI 자동 클릭/좌표 자동화는 OUT.

`SLSK-003 [MUST]` **slskd Full Mode**: slskd의 HTTP API를 통한 연결 확인, API Key, 검색, 검색 결과 파일/유저/경로/크기, 사용자 공유 파일 탐색, 다운로드 요청, 진행률, 완료 이벤트 수신 또는 polling, 완료 파일 경로 확인 및 Inbox 등록. slskd 미설치는 전체 앱 사용에 영향을 주지 않는다.

`SLSK-004 [MUST]` slskd 결과는 peer 상태·대기·전송 실패·오프라인·경로 없음·다운로드 재시도를 명확히 표시하고 응답의 파일명을 안전하게 검증한다. 적법한 소스만 수집하도록 사용권 안내 및 검색 사용 범위 설명.

### 09.2 검색·표시

`SLSK-005 [MUST]` 검색 입력, 결과 갱신, 사용자 이름, 공유 경로, 이름, 포맷, 크기, 제공된 비트레이트, `Download/Browse/Filter`, 전송 상태. 같은 제목의 서로 다른 사용자·음원을 하나의 파일로 오인하지 않는다. 가능한 경우 일괄 파일·폴더 선택 지원.

`SLSK-006 [MUST]` 소스에 표준 URL이 없는 경우 가짜 웹 주소를 만들어 내지 않는다. `sourceType=soulseek`, username, peer path 등은 **Library sidecar provenance**에만 저장할 수 있다. URL-only인 History에는 **유효한 링크를 사용자가 별도로 제공하지 않는 한 추가하지 않는다.**

### 09.3 기능별 현실적 수준

| 항목 | SoulseekQt Watch | slskd API |
|---|---|---|
| 설치·연결 상태 | 폴더/프로세스 기준 | API 기준 |
| 내부 검색 | 제공 안 함 | 제공 |
| 다운로드 요청 | 제공 안 함 | 제공 |
| 완료 감지 | 파일 안정화 감시 | 이벤트/polling + 파일 검증 |
| 파일 import | 예 | 예 |
| 기존 사용자 데이터 변경 | 금지 | 사용자가 요청한 전송만 |

# 10. 선택적 입력 확장: Local / Archive / Web Capture

`WEB-013 [MUST]` Local file·폴더를 직접 Inbox 또는 Editor에 가져온다. 원본을 그대로 참조하는 경우와 Library로 복사하는 경우를 명시적으로 선택. 파일 경로 Unicode·네트워크·외장 볼륨 분리·권한 오류를 지원한다.

`WEB-014 [FUTURE]` ZIP/TAR 등 아카이브 검사·압축 해제·오디오 목록·선택 가져오기. Zip Slip(path traversal), zip bomb, 암호화 아카이브, 실행 파일 자동 실행 위험 보호. 원본 아카이브가 삭제되어도 Library 샘플은 유지.

`WEB-015 [FUTURE]` Web Capture: 허용된 DRM 없는 웹페이지 오디오에 대해 사용자가 명시적으로 `Record/Stop`을 조작하고 파일을 Inbox로 보낸다. 브라우저 미리보기·레벨 미터·record duration·트림 가능. **yt-dlp 차단을 자동 DRM 우회로 연결하지 않는다.** OS별 오디오 캡처 장치 접근권한·서비스 약관 준수 필수.

`WEB-016 [FUTURE]` Internet Archive, Freesound 및 직접 HTTP 파일을 추가 Adapter로 제공하되 각각 권한과 rate limit, 라이선스 필드, 실제 다운로드 가능한 media 여부 확인. Freesound 등에서 API Key/계정이 필요할 경우 **선택적**이며 필수 유료 API 조건은 안 됨.

# 11. INBOX: 임시 오디오 작업 공간

`INB-001 [MUST]` Inbox는 **다운로드 완료 또는 사용자가 가져온 원본/후보 파일**을 보여준다. Library와 다르며 자동 영구 보관소가 아니다. WAV 추출 여부와 상관없이 원본 파일은 별도 관리한다.

`INB-002 [MUST]` List 필드: 이름, 원본 파일 경로, source adapter, 포맷, length(알면), size, import/ready/failed/missing 상태. `ALL/WEB/TORRENT/SOULSEEK/LOCAL` 필터, 이름·형식·날짜(작업 기록의 날짜) 정렬, 폴더 expand, 큰 리스트 가상화, multi-select.

`INB-003 [MUST]` 동작: `Play`, `Open Editor`, `Add to Library`, `Bulk Add`, `Copy Path`, `Reveal File`, `Remove from Inbox`, `Delete actual file…`(별도 경고), `Retry Import`. 선택한 범위만 최종 sample로 export 가능. URL별 다운로드 완료 후 Inbox 자동 등록은 설정으로 끌 수 있다.

`INB-004 [MUST]` 미완성 `.part`, 다운로드 중이거나 다른 프로그램이 기록하는 파일은 `Pending`으로 보관/미리보기 비활성. 파일 stable-size 검사+가능한 경우 디코더 검증. 파일명만 오디오여도 실제 코덱 미지원 시 `Unsupported` 표시.

`INB-005 [MUST]` 원본 데이터 삭제 정책은 source 별: P2P 원본 절대 자동 삭제 안 함, WEB 임시 소스 캐시는 사용자 cache 정책 따름, Local 원본 무단 변경 안 함. `Remove from Inbox`는 DB/표시 목록 제거이며 실제 파일 삭제와 혼동하지 않는다.

`INB-006 [FUTURE]` 다중 선택 후 일괄 format convert, 공통 태그 적용, 샘플 추출·자동 슬라이스, 폴더 구조/앨범 트랙 정보 파싱, 컬렉션 편입.

# 12. LIBRARY: 개인 샘플 데이터베이스

### 12.1 기본 의미

`LIB-001 [MUST]` Library는 **실제로 보유한 오디오 파일과 소스·태그·설명에 대한 조회 가능한 인덱스**다. 원본 소스가 삭제/오프라인이어도 최종 sample WAV 등의 파일이 보존되어 있으면 재생·편집할 수 있다.

`LIB-002 [MUST]` 영구 샘플 옆에 텍스트 `*.sample.json` Sidecar를 보관하고, Library 전체 검색용 `SQLite` 인덱스는 **파생 자료**로 취급한다. SQLite 파일이 사라지면 Sidecar를 스캔해 재구축한다.

### 12.2 샘플 사용자 메타데이터

`LIB-003 [MUST]` `Name` 자유 입력, 파일명과 별개 title 지정 가능. `Tags` 0개 이상 자유롭게 추가·삭제·이름 변경·일괄 추가, `Description` multiline, Unicode/한글/특수기호/이모지 포함 원문 보존. 태그는 쉼표/Enter로 확정, 중복 태그 비교에는 trim/Unicode normalization 정책 적용(원래 보이던 사용자 문자열은 보존 가능). 태그 입력 시 존재 태그 자동완성.

`LIB-004 [MUST]` Note/Description의 취소, Undo/Redo, 저장 진행 표시, 프로그램이 꺼져도 글을 잃지 않는 draft policy. 변경을 저장하기 전 Library 파일을 오염시키지 않는다. 위험한 충돌은 merge/keep mine/keep remote 등 해소 UI.

`LIB-005 [MUST]` 원본 metadata와 사용자 메모 분리. 제목/아티스트/앨범/원본 링크 등 웹 측 값도 사용자가 수정할 수 있지만, 사용자 수정값과 fetched 값이 뒤섞여 실제 출처가 사라지지 않아야 한다.

### 12.3 탐색·정렬·검색

`LIB-006 [MUST]` List(이름, waveform peak thumbnail, duration, tags, source, format), 이름/태그/Description/출처 검색, Filter(형식, 채널, 길이, sample rate, 출처), 정렬(이름, 길이, size, last added, source), 비어 있는 결과 안내, 최근 사용/최근 저장. 검색 인덱스와 텍스트 검색 누락 여부 테스트.

`LIB-007 [MUST]` 샘플 직접 미리듣기, Editor 재열기, 원본 URL/파일 열기, file drag, copy path, reveal, rename, copy/duplicate, delete from index만, delete actual file(확인), 전체 여러 파일 내보내기. 삭제 후 복구 가능한 백업을 제공할 수 있으나 무단 자동 삭제 금지.

`LIB-008 [MUST]` **Missing/Changed File**: 파일 이동·이름 변경·외장 SSD 분리 감지. 파일 해시/크기·mtime·Sidecar id로 다시 연결, 사용자가 새 경로 선택, DB 경로 수정. 손상된 오디오와 단순 누락은 분리해 표시.

### 12.4 다시 편집·기타

`LIB-009 [MUST]` 기존 샘플을 다시 잘라 `Save as New`, `Overwrite (확인)`, `Duplicate` 가능. 이미 Trim한 샘플을 다시 자르는 경우 선택 시간은 해당 샘플의 로컬 0기준이며 source original 위치와 혼동하지 않음. 가능한 경우 original provenance에서 누적 타임코드 계산.

`LIB-010 [SHOULD]` 비트레이트·샘플레이트·채널·길이·BPM/key(분석한 경우)에서 필터. 즐겨찾기/별점(선택), 여러 샘플 공통 설명·태그, duplicate hash display, imported file auto-index.

`LIB-011 [FUTURE]` Collections, Smart Collections, similarity search, random sample playback, loop previews, bulk metadata export, 파일을 움직이지 않는 external library reference, multi-library, 외장 저장소 reindex, DAW 사용 기록(자동 보장 X), BPM/Key 분석 결과 이용.

# 13. HISTORY: URL / Tags / Description만

### 13.1 가장 엄격한 계약

`HIS-001 [MUST]` **History 항목을 디스크에 영구 저장할 때 기본 구조는 반드시 아래 세 필드뿐이다.** 음원 파일, 재생 기록, waveform, 날짜, 사운드 분석, 이미지, 파일 경로 등은 포함하지 않는다.

```json
{
  "source": "https://www.youtube.com/watch?v=example",
  "tags": ["metal", "percussion", "field-recording"],
  "description": "문을 닫는 소리처럼 들리는 금속성 잔향"
}
```

다중 항목은 위 객체의 JSON 배열. JSON 구조를 확장하려면 **사용자 승인**이 먼저 필요하다. 다운로드 시각을 UI에서 보여주기 위한 값이 필요하다면 History가 아닌 Task·Library DB 영역에서만 관리할 수 있으며, History JSON의 계약을 깰 수 없다.

`HIS-002 [MUST]` History에 들어갈 `source`는 사용자가 다시 열 수 있는 실재 URL이다. 원본 URL이 없는 Soulseek나 Local 파일은 자동 History 대상으로 삼지 않고 provenance Sidecar에서만 관리한다. 가짜 링크 생성 금지.

`HIS-003 [MUST]` 저장 조건: 사용자가 `Save to History`를 선택하거나 `Save Sample` 시 설정에서 History 저장을 켰을 때. 단순 URL 조회·검색·미리듣기만으로 무조건 자동 기록하지 않는다.

### 13.2 검색·수정·중복

`HIS-004 [MUST]` 검색(전체 URL/Tags/Description), 필터(Tag AND/OR), 기록 편집, 기록 삭제, 모든 기록 삭제(강한 확인), 원본 열기, URL 복사, WEB 탭으로 다시 LOAD, JSON Import/Export, 오프라인 목록 읽기.

`HIS-005 [MUST]` 같은 URL에서 여러 샘플을 만들면 `Keep separate / Merge Tags and Description / Replace / Cancel` 선택. 원본 URL normalization은 추적 파라미터만 제거하고 서로 다른 타임스탬프·버전의 실제 소스를 잘못 합치지 않도록 한다.

`HIS-006 [MUST]` History 삭제해도 Library 샘플과 Sidecar 보존. Library 파일을 지워도 History 보존. History 누락/손상 시 파일 전체를 초기화하지 말고 구제·백업 가능 부분을 우선 보존한다. 자동 동기화로 반대쪽 데이터를 지우지 않는다.

### 13.3 History UI

`HIS-007 [MUST]` 각 row에 URL 축약, 태그 chip, Description 2~3줄 preview, full detail overlay, `OPEN/LOAD/EDIT/DELETE`. 출처 아이콘을 사용할 수 있지만 History JSON에 플랫폼 정보는 추가 저장하지 않는다. 긴 URL은 복사·툴팁 지원. 중복 URL 행은 정책에 따라 나란히 표시.

# 14. 파일 시스템·저장·데이터 규격

### 14.1 사용자 경로

`DAT-001 [MUST]` 세 가지 Export 모드: **Global Library**, **Current Project Samples**, **Custom Folder**. 기본 예시 Windows `%USERPROFILE%\Music\Samples\tokkebi`, 사용자가 원하면 `C:\Samples`; macOS `~/Music/Samples/tokkebi`. AppData/Library 같은 설정 폴더와 실제 음원 보관 위치는 분리한다.

`DAT-002 [MUST]` 프로젝트별 경로는 plugin State에 저장. DAW 프로젝트 실제 경로 자동 탐색은 host-specific 검증 후 조건부 제공; 수동 Browse가 항상 있어야 한다. 상대 경로가 가능하면 프로젝트 이동에 대응하지만, DAW 내부 패키지 경로를 임의 수정하지 않는다. Global fallback은 사용자 승인 후 적용.

`DAT-003 [MUST]` 파일 이름 정규화(Windows reserved names, Unicode, invalid characters, 중복), 파일명 중복이면 `name`, `name_02`, ... 생성 또는 사용자 결정. 상대 경로 traversal, symbolic link, 외부 볼륨, read-only, 공간 부족 검사.

### 14.2 저장 구성 예시

```text
<User chosen Library Root>/
  Samples/
    metallic_hit.wav
    metallic_hit.sample.json
    voice_001.flac
    voice_001.sample.json
  library.sqlite
  history.json

<OS per-user application-data>/tokkebi/
  config/settings.json
  db/jobs.sqlite
  cache/originals/
  cache/waveforms/
  cache/preview/
  logs/
  tools/yt-dlp/<version>/
  tools/ffmpeg/<version>/
  tools/deno/<version>/
  backups/
```

`DAT-004 [MUST]` 최종 샘플과 Sidecar를 함께 저장하되 파일별 ID를 통해 rename·move를 추적한다. Library SQLite는 빠른 색인 역할; wave peak cache·download queue는 별도. History는 Library root 또는 사용자가 지정한 backup 가능한 JSON 경로에 보관한다.

### 14.3 Sidecar 버전 1 제안

```json
{
  "schemaVersion": 1,
  "id": "uuid-sample",
  "file": "metallic_hit.wav",
  "name": "metallic_hit",
  "tags": ["metal", "field-recording"],
  "description": "긴 잔향",
  "source": {
    "kind": "web",
    "originalInputUrl": "https://open.spotify.com/track/example",
    "downloadUrl": "https://youtube.com/watch?v=example",
    "originalFilename": "sample-source.webm",
    "sourceArtist": "Artist",
    "sourceTitle": "Title",
    "sourceUploader": "Artist - Topic"
  },
  "selection": {
    "startSeconds": 73.25,
    "endSeconds": 79.0,
    "timebase": "original-source"
  },
  "audio": {
    "format": "wav",
    "sampleRate": 44100,
    "bitDepth": "pcm24",
    "channels": 2,
    "durationSamples": 253575,
    "sha256": "<computed-hash>"
  }
}
```

P2P와 Local은 `kind`, `sourcePeer`, `sharePath`, `infoHash`, `localImportPath` 등 각자 필요한 provenance만 사용한다. 존재하지 않는 링크를 `downloadUrl`에 넣지 않는다. 추가 분석(BPM/key 등)은 명시적으로 버전된 optional 필드에 보관할 수 있다.

`DAT-005 [MUST]` 자동 생성 결과와 사용자가 직접 입력한 원본 텍스트를 구별. `source.originalInputUrl`과 실제 오디오를 얻은 `downloadUrl`을 혼동하지 않음. 출처 수정 시 provenance integrity 경고. 개인 접근 토큰이 들어간 URL은 장기간 저장하지 않거나 secret query 삭제 정책 적용.

### 14.4 DB·백업·원자성

`DAT-006 [MUST]` SQLite 인덱스는 WAL, schema migration, 동시 접근/락 충돌, 재시작 복구, DB corruption detect, sidecar rebuild, orphan sidecar scanning을 갖는다. 모든 데이터 스키마 변경은 구버전 파일 migration test가 필요하다.

`DAT-007 [MUST]` 임시 `*.tmp` 작성·fsync 가능한 범위의 검증 후 atomic replace; **오디오 생성 완료가 Sidecar/DB 반영보다 늦거나 빨라지는 경우에도 recover 가능**해야 한다. 여러 파일의 트랜잭션을 완벽한 원자 연산이라 가장하지 않고 recovery journal을 남긴다.

`DAT-008 [MUST]` Local file/원본 WEB cache/Soulseek·Torrent 원본/Library export를 다른 보존 영역으로 관리. P2P 수집 원본에 대해 delete-original 옵션 기본 OFF 및 자동 삭제 OUT. `Uninstall`은 사용자 라이브러리 데이터를 보존한다.

`DAT-009 [MUST]` 오디오 hash(SHA256 등)로 중복 확인, 이름/파일크기만으로 동일성 확정 금지. 많은 음원은 일정 해시 계산을 백그라운드에서 하며 변환·저장 중에는 파일 바이트 일관성을 유지한다.

### 14.5 캐시 정책

`DAT-010 [MUST]` Preview/Waveform/Original cache 각각 최대 사용량·정리 전략, LRU 또는 보존 기간, 수동 지우기 버튼, 현재 작업에서 사용 중인 파일 보호. Saved Sample은 어떤 캐시 정리에서도 삭제하지 않는다.

`DAT-011 [MUST]` 캐시가 외장 디스크 분리·파일 잠금·권한 오류로 삭제 실패했을 때 다음 실행으로 미룬다. 백그라운드 다운로드 도중 파일을 지우지 않는다. UI에 Cache/Library/Inbox 용량을 혼합 표시하지 않는다.

# 15. DAW 통합: FL Studio / Ableton Live / Logic Pro

### 15.1 공통 외부 파일 드래그 계약

`DAW-001 [MUST]` 사용자에게 보이는 `DRAG TO DAW`는 (1) 완성·읽기 가능한 출력 파일을 준비하고 (2) 운영체제의 실제 **파일 Drag & Drop**을 실행한다. 내부 문자열 링크, 가짜 파일명, 재생 버퍼 포인터를 드롭하지 않는다.

`DAW-002 [MUST]` Drag 가능한 곳: Editor의 출력 파형, Inbox ready 파일, Library row, 선택한 여러 파일. 마우스다운 → 파일 준비(필요 시 렌더링 표시) → 외부 Drag. 렌더링 중인데 파일이 아직 없으면 drag cursor를 제공하지 않으며 `Preparing Sample...` 상태를 보인다.

`DAW-003 [MUST]` 호스트별로 drop target에 제약이 있을 수 있다. 실패 시 `Floating Drag Window`(독립 Helper) → `Reveal in Explorer/Finder` → `Copy Path`의 세 대안을 제공한다. 직접 삽입 성공으로 UI가 거짓 안내하는 것을 금지한다.

`DAW-004 [MUST]` 플러그인 unload/프로젝트 종료 후 파일 참조: 프로젝트 폴더와 전역 Library의 사용 파일은 안정적 위치에 저장. 임시 Cache만 Drag로 호스트에 전달하지 않는 것을 원칙으로 하며, 임시 Drag export를 허용할 경우 안전한 만료·복사 안내 필요.

### 15.2 호스트별 대상 테스트

- **FL Studio Windows/macOS:** Playlist, Channel Rack/Sampler, Edison이 외부 파일 Drag를 받는지 개별 확인. Plugin이 focus를 잃었을 때 OS Drag 흐름 테스트. 프로젝트 저장·재로드 경로 복원 확인.
- **Ableton Live Windows/macOS:** Arrangement Clip 영역, Session Slot, Simpler/Drum Rack 브라우저 등 개별 Drop 대상; host의 file copying/Collect All and Save와 충돌 여부.
- **Logic Pro macOS:** AU Validation 통과, Tracks Area, Quick Sampler, Sampler drop 실험. `.logicx` 패키지 내부 접근/쓰기 허가 없이 프로젝트 복사를 수행하지 않는다.

`DAW-005 [CONDITIONAL]` DAW 내 직접 clip 생성, transport position 삽입, BPM 자동 공유, DAW project path 자동 감지는 **호스트별 공식 API가 허용되는 범위에서만** 향후 개발. Plugin 공통 API로 자동 보장하지 않는다.

### 15.3 프로젝트 설정·State

`DAW-006 [MUST]` 플러그인 preset/host project에 현재 탭, 마지막 로드 source reference, draft Name/Tag/Description(민감정보 정책에 따름), Time selection, 프로젝트 Save Folder, UI scale, 일부 탭 상태를 compact한 state로 저장한다. 오디오 파일 전체 또는 거대한 Cache를 호스트 state에 삽입하지 않는다.

`DAW-007 [MUST]` 인스턴스별 Project Folder와 상태는 분리하고, Global Library, download queue, history는 Helper/사용자 profile에서 공유. 프로젝트 저장·undo·host preset load·offline render·sample rate 변경·host crashing case 테스트.

### 15.4 오디오 패스스루

`DAW-008 [MUST]` 이 앱은 기본적으로 샘플 수집 유틸리티다. FX 형식으로 호스트 오디오가 들어오면 원 신호는 자동 변경하지 않으며 Preview mix은 사용자에게 명확히 표시. 플러그인을 인서트했다고 오디오가 갑자기 무음·증폭·단절되지 않게 한다. bypass 시 정상 passthrough. 오디오 미리듣기 장치/호스트 sync는 세부 테스트.

# 16. 프로그램 내부 아키텍처

### 16.1 선택 기술

`ARC-001 [OPEN/RECOMMENDED]` C++20, iPlug2(IPlug/IGraphics), CMake, SQLite, JSON, FFmpeg/FFprobe, yt-dlp 외부 프로세스, Windows NSIS 및 macOS PKG. iPlug2는 VST3, AUv2, standalone을 지원하고 zlib-like 라이선스이므로 유료 GUI framework 라이선스 회피 측면에서 유리하다. 최종 선택은 P00 Windows/macOS 실빌드·외부 Drag 가능성 검사 뒤 확정.

- GUI와 plugin은 가능한 한 같은 ViewModel을 사용, model/controller는 host에 종속시키지 않는다.
- FFmpeg·yt-dlp·qBittorrent·slskd 어댑터는 `Helper`에서 실행한다. Plugin 프로세스 내부에 동기 shell을 넣지 않는다.
- CMake targets: `core`, `ui`, `plugin_vst3`, `plugin_au`, `standalone`, `helper`, `tests`. 실제 iPlug2 target명은 upstream 문서에 맞춘다.
- Dependency pinning + license SBOM, `third_party` 출처 문서화, 취약점 업데이트 경로 마련.

### 16.2 프로세스 모델

```text
FL Studio / Ableton / Logic
     │  VST3 or AU
     ▼
[PLUGIN UI + Preview DSP] ── IPC ── [PER-USER HELPER]
                                         │
               ┌─────────────────────────┼───────────────────────┐
               ▼                         ▼                       ▼
          yt-dlp / ffmpeg          qBittorrent API           slskd API
               │                         │                       │
               └─────────────────────────┼───────────────────────┘
                                         ▼
                                [INBOX + LIBRARY]
                                         │
                              WAV + JSON + SQLite
```

`ARC-002 [MUST]` Helper는 사용자 세션에 단일 인스턴스 우선, Plugin/Standalone 여러 프로세스가 동일 Helper와 IPC로 연결된다. Helper가 없는 경우 안전하게 실행·재연결하며 host에서 종료까지 대기하지 않는다. GUI 종료와 download job 종료는 분리하되 사용자가 전체 Helper 정지 가능.

`ARC-003 [MUST]` IPC는 Named Pipe(Windows), Unix Domain Socket/XPC 검토(macOS) 등 사용자 계정별 접근 통제 가능한 로컬 전용 수단. 버전 협상, auth token/OS ACL, requestId, cancellation, timeout, async progress events, duplicate event idempotence를 갖춘다. localhost HTTP를 쓸 경우 loopback 제한·인증·CORS/CSRF 대응 필수.

`ARC-004 [MUST]` Helper crash → Plugin 정상 유지, 상태 `Disconnected`, 재시작·재연결. Plugin crash → Helper job 정상 관리, 재실행 시 상태 복구. Helper가 종료되면 파일 rename·DB commit 중이더라도 재실행에서 복원 또는 보고.

### 16.3 공통 API(예시)

```text
getHealth(); getCapabilities(); getToolVersions()
resolveSource(input); searchSource(query,sourceAdapter)
listSourceEntries(sourceId); previewAudio(sourceId, range)
queueDownload(sourceId, options); getTask(taskId); listTasks()
cancelTask(taskId); retryTask(taskId)
getInbox(filter); importLocalFile(path); openEditor(itemId)
renderSample(itemId, selection, audioOptions, destination)
listLibrary(query); saveMetadata(sampleId, patch, expectedVersion)
rebuildLibraryIndex(); verifyLibrary(); exportHistory(); importHistory()
listHistory(query); saveHistory(entry); deleteHistory(entryIdOrIndex)
getSettings(); patchSettings(); getSourceAdapterStatus()
```

이 API는 구현 시점에 계약 테스트로 고정한다. 예시는 요구되는 기능을 나타내며 정확한 RPC 명칭은 별도 IDL에 정의. 모든 요청은 `Success/Error(code, message, detail)` 형태를 가져야 하고, 오류 코드가 정상 완료 상태로 위장되지 않아야 한다.

### 16.4 SourceAdapter

`ARC-005 [MUST]` SourceAdapter의 capability flags: `canResolve`, `canSearch`, `canList`, `canPreview`, `canDownload`, `supportsRange`, `supportsPause`, `providesProgress`, `providesOriginUrl`. 지원하지 않는 메서드는 명시적 `UNSUPPORTED`를 반환하고 UI는 사용할 수 없는 액션을 표시한다.

Adapter 종류: `YT_DLP`, `DIRECT_HTTP`, `SPOTIFY_RESOLVER`, `APPLE_RESOLVER`, `YOUTUBE_SEARCH`, `MUSICBRAINZ`, `QBITTORRENT`, `SOULSEEK_QT_WATCH`, `SLSKD`, `LOCAL_FILES`, FUTURE `WEB_CAPTURE`, `ARCHIVE`, `INTERNET_ARCHIVE`, `FREESOUND`.

### 16.5 비동기 작업과 안전한 파일 쓰기

`ARC-006 [MUST]` Request/Result/Event를 스레드 세이프한 큐로 전달하며 UI 쓰레드에만 뷰 변경을 적용한다. 파일 디코딩과 DB 작업은 worker threads에서 실행. 오디오 콜백에서는 thread-safe immutable/snapshot 데이터를 활용하고 blocking mutex 대기 금지.

`ARC-007 [MUST]` 백그라운드 cancel 시 native process tree 관리, 부분 파일 복구 정책, stdout/stderr 제한, 좀비 프로세스 방지. 외부 명령 실행 시 `system()` 문자열 조합으로 쉘에 넘기지 않고 shell injection 방지와 argv 분리 전달.

# 17. 의존성 설치, 탐지, 업데이트

### 17.1 실행 시 필요한 외부 프로그램

| 도구 | 목적 | 기본 정책 |
|---|---|---|
| yt-dlp | 웹 미디어 인식·추출 | 설치본 자동 탐색 / 앱 관리본 / Browse |
| FFmpeg | Decode/Convert/Trim | 동일 |
| FFprobe | 정확한 오디오 정보 | FFmpeg와 함께 버전 확인 |
| Deno | yt-dlp YouTube challenge runner | 설치본 탐색 / 앱 관리본 / Browse |
| yt-dlp-ejs | YouTube extractor 보조 | 공식 바이너리에서 동봉 여부 확인, Python/packager별 점검 |
| qBittorrent | torrent | 별도 설치 선택 연동 |
| SoulseekQt | 폴더 감시 | 별도 설치 선택 연동 |
| slskd | HTTP API 검색·다운로드 | 별도 설치 선택 연동 |

`DEP-001 [MUST]` 실행 파일 탐색 순서: 사용자 명시 경로 → 이전에 저장한 정상 경로 → PATH → 알려진 per-user package manager 경로 → 앱 관리본. `C:\...\Desktop\ffmpeg\bin` 같은 독자 설치 경로는 자동 발견 실패 가능성이 있으므로 반드시 `Browse` 제공. 잘못된 파일·권한 없거나 버전 미달은 경고.

`DEP-002 [MUST]` `--version`/capability probe와 작은 진단 작업을 실제 실행. 단순 파일 존재 확인만으로 Ready 표시 금지. ffprobe, ffmpeg, yt-dlp·Deno의 architecture/권한/실제 연동을 확인한다.

`DEP-003 [MUST]` `Use Existing`을 선택하면 프로그램이 시스템 파일을 삭제하거나 자동 업데이트하지 않는다. `App Managed`는 사용자 AppData/Application Support 아래 버전별 격리 공간에 설치한다. `Manual Path`는 사용자가 파일 선택. 설정에서 모드를 바꿀 수 있고 이전 모드는 유지.

`DEP-004 [MUST]` yt-dlp 공식 PyInstaller/zipimport/PyPI 방식에 따라 EJS 구성 여부가 다를 수 있으므로 `yt-dlp-ejs`와 지원 JS runtime을 검사한다. Deno 권장·최소 2.3.0 여부를 버전 검사하되 향후 upstream 요구에 맞게 정책을 갱신한다. 자동으로 remote npm script를 실행하지 않고 필요한 다운로드는 명시적 정책/동의로 처리.

`DEP-005 [MUST]` update manager: 공식 HTTPS 배포 주소, 체크섬·서명 검증, unpack의 sandbox, 새 버전 진단 테스트, 성공 후 atomic version switch, rollback 가능, 사용 중 job 중단 방지, 신뢰되지 않는 미러 사용 금지. 이전 설치 경로와 user settings 보존.

`DEP-006 [MUST]` macOS 서명·공증된 App/VST3/AU 내부를 런타임 업데이트가 변조하지 않는다. 외부 실행 파일은 앱의 격리된 writable tool store에서 관리하고 Gatekeeper·quarantine·실행 권한 이슈를 검증한다. Windows Defender·SmartScreen false positive 가능성을 사용자에게 정직하게 안내.

`DEP-007 [SHOULD]` 완전 설치형(Full, 재배포 라이선스 준수)과 소형 설치형(Minimal, 처음 실행 시 기존 설치 감지/동의 후 공식 도구 받기) 옵션. **기본 제안: Full 제공 + 기존 버전 사용 선택**. 번들 외부 라이선스 검토 결과에 따라 배포 방법을 변경 가능.

# 18. Installer / First Run / Uninstall / Release

### 18.1 목표

`INS-001 [MUST]` 사용자에게 터미널·개발툴 설치를 강요하지 않고 운영체제용 installer 실행 후 Standalone + Plugin을 사용할 수 있는 제품으로 배포한다. 실패한 외부 도구 연결도 WEB의 일부 기능만 비활성화해야 하며 Library/History는 정상 작동해야 한다.

### 18.2 Windows 설치

`INS-002 [MUST]` NSIS를 기본 후보로 설정: 설치 화면(언어, 라이선스, 설치 항목, 개인/전체 사용자, 경로, Summary), `Standalone.exe`, `Helper.exe`, `VST3`의 규격 경로 설치, 시작 메뉴 바로가기(옵션), 언인스톨 항목, 설치 로그, 복구. 파일 관리자 context 메뉴·자동 실행·방화벽 권한 등록 등은 **기본 OFF**.

Windows VST3 표준 시스템 설치 경로 예: `%COMMONPROGRAMFILES%\VST3`; per-user 경로는 host가 탐지하는 표준 범위와 실제 테스트로 결정한다. 사용자에게 잘못된 임의 plugin 경로를 무조건 쓰도록 요구하지 않는다.

`INS-003 [MUST]` 설치 후 자동 외부 도구 탐지는 Standalone 최초 실행 시 진행; 설치 도중 긴 네트워크 다운로드가 필수가 아니게 한다. 사용자 데이터는 AppData 및 선택한 Library 위치에 저장해 Program Files 쓰기 요구 금지.

### 18.3 macOS 설치

`INS-004 [MUST]` `pkgbuild/productbuild` 또는 정책에 맞는 서명 가능한 패키지: `/Applications` Standalone 앱, `VST3` 표준 경로, `/Library/Audio/Plug-Ins/Components` AUv2(전체 사용자)/적절한 per-user 대안, Helper 앱. install scripts는 최소화. Intel x86_64/Apple Silicon arm64를 실제 검증.

`INS-005 [MUST]` 개발 빌드(ad-hoc/unsigned)와 **대중 배포용** `Developer ID` 서명·Notarization·stapling을 구별한다. 서명 자격증명은 CI Secrets에만 두고 repo 커밋 금지. 코드 서명 비용/계정 부재는 Release Blocker로 표시하되 개발용 빌드를 배포 완료로 보고하지 않는다.

`INS-006 [MUST]` AU는 Logic의 Plug-in Manager 검사/`auval`, plugin rescan, quarantine, Library permissions, separate Helper process, macOS 보안/파일 접근 허용 테스트가 필요하다.

### 18.4 첫 실행 마법사(정확한 순서)

1. 환영/프로그램 목적/데이터 로컬 저장 안내.
2. `Create New Library / Choose Existing Library` (폴더 writable 검사, sidecar/schema 호환성 탐지).
3. yt-dlp/FFmpeg/FFprobe/Deno **Existing / App Managed / Browse** 선택; 발견된 프로그램의 경로·버전·Ready/Outdated/Fail 표시.
4. 설치/사용에 동의하면 필요한 app-managed 도구를 신뢰 가능한 원본에서 가져오고 검증한다. 이미 있는 파일을 자동 덮어쓰기 금지.
5. P2P는 `Skip / Connect qBittorrent / Watch SoulseekQt Folder / Connect slskd`로 선택적. 설치되지 않아도 다음 단계 가능.
6. 오디오 출력 환경/Preview Volume 확인(Standalone), Plugin은 host 사용 안내.
7. Download destination: Global, Project Folder 정책, 기본 format WAV 24-bit/44.1kHz 또는 `Keep Source`.
8. Test Diagnostics 요약(플러그인/오디오/서브 프로세스/권한), 실패는 자세한 `Fix` 링크.
9. Finish → WEB 또는 Library. 첫 실행 wizard 취소 시 Read-only Library 동작 등을 최대한 허용.

`INS-007 [MUST]` 마법사를 지나가도 Settings의 `Run Setup Again / Diagnostics`에서 같은 설정을 재실행. 외부 qBittorrent/slskd 프로그램은 사용자의 명시적 설치·실행 상태만 활용한다.

### 18.5 인스톨러 선택지

- **Full Installer:** 앱에 필요한 OSS 실행 도구를 함께 포함하되 정확한 별도 라이선스 및 해당 바이너리의 소스 제공 의무 이행.
- **Minimal Installer:** 앱과 Plugin/Helper만 설치, 최초 실행 시 기존 도구 선택 또는 app-managed 도구의 사용자 동의에 따른 다운로드.
- 설치 크기와 인터넷 요구사항, Windows/macOS 패키지 지원 범위 명시. 두 설치본 간 상호 전환·업데이트 후 사용자 설정 보존.

### 18.6 설치 제거

`INS-008 [MUST]` 앱 본체·플러그인·Helper 제거 옵션, 현재 진행 중 작업 안전한 정지, 플러그인 이미 로드된 경우 안내. **Library WAV/FLAC, Sidecar, History JSON, 개인 작업 파일, 기존 외부 yt-dlp/FFmpeg/qBittorrent/Soulseek는 기본 보존.**

`INS-009 [MUST]` 설정·캐시 삭제는 명시적 체크박스(기본 OFF)로 분리. `Remove all personal sample data` 같은 위험한 옵션은 **기본 제공하지 않는 것**을 원칙으로 하거나 별도 2단계 확인을 요구한다. 소스 클라이언트·외부 프로그램의 사용자 데이터는 절대 제거하지 않는다.

### 18.7 버전·업데이트·배포 파이프라인

`REL-001 [MUST]` SemVer, `APP_VERSION`, IPC Protocol Version, Library Schema Version, Sidecar Schema Version, History Schema(고정 3-field), Adapter API Version을 분리 기록. 호환성 문제를 감지하면 migration/읽기 전용 등 안전한 방법을 제공.

`REL-002 [MUST]` GitHub Releases에 Windows installer와 macOS package, SHA256 checksum, release notes, OSS license bundle, 테스트 환경/미지원 항목/known issues 게시. 개발·nightly·beta·stable 채널 구분.

`REL-003 [MUST]` 자동 업데이트(앱)은 검증된 자체 새 Release로만; Helper와 Plugin의 호환 버전을 확인하고 DAW 프로젝트가 열려 있을 때 plugin DLL/Bundle을 강제로 덮어쓰지 않는다. 별도 외부 도구 업데이트와 분리한다.

`REL-004 [MUST]` 어떤 파일·체크섬·서명·출처를 배포했는지 재현 가능한 manifest와 SBOM/라이선스를 생성한다. Release artifact가 빌드에 실패했거나 서명이 없으면 정식 Ready로 표시하지 않는다.

# 19. Settings: 세부 옵션 백과

`UI-008 [MUST]` Settings는 좌측 카테고리 + 우측 내용 패널로 설계. 현재 값, default, 변경 상태, 적용(필요시), restore default 및 부작용 안내. 프로젝트 고유값과 전역값을 시각적으로 분리. 저장 실패는 값이 적용된 척 하지 않는다.

| 그룹 | 설정 항목(최소) |
|---|---|
| **General** | UI 언어 ko/en, light/dark, pixel/grain 강도, UI scale, compact mode, start tab, 최근 탭 복원, tooltips, reduce motion, 파일 탐색기 열기, 알림 on/off, 버전/도움말 |
| **Input & Search** | URL 붙여넣기 방식, clipboard auto detect(OFF), 검색 결과 개수 5/10/20, 자동 강조 threshold, MusicBrainz 보강 허용, metadata cache 기간, Spotify/Apple resolver enable/disable |
| **Downloads** | yt-dlp binary mode/path, ffmpeg/ffprobe path, Deno path, 외부 도구 버전, 동시 downloads, retries, timeout, cache, priority, per-domain limit, bandwidth cap, logs, auto Inbox import |
| **Audio** | WAV/AIFF/FLAC default, bit depth, sample rate, channels, Preview gain, loop/stop behavior, fade length, normalize, trim silence, DC offset 제거, 분석 auto-on/off, native source preservation |
| **Storage** | Global Root, Project Samples path(프로젝트별), custom export, filename pattern, suffix collision policy, backup location, source cache location, quota, temporary retention, verify paths |
| **Library** | reindex, Verify Files, Link missing files, duplicate hash scan, show Sidecar, description autosave, tags autocomplete, sort order, backup and restore, collections(FUTURE) |
| **History** | History recording option, separate vs merge duplicate URLs, JSON export/import, clear history with confirmation, history location, format validator |
| **P2P/qBittorrent** | WebUI host/port/auth, connect/test, app tag/category, allow manage created jobs, destination viewing, connection status |
| **P2P/SoulseekQt** | watched folder, recursive watch, rescan now, watch enable, stable file delay, audio-only import |
| **P2P/slskd** | API URL/key, connect/test, download folder, search filter, completed import on/off |
| **Advanced** | Helper status/start/stop, IPC diagnostics, logs/verbose, GPU renderer, waveform cache quality, migration check, DB integrity, reset UI only, reset tools config only, clear cache, show third-party license, check for updates |

`UI-009 [MUST]` Setup/Dependencies 탭에는 프로그램별 `Not found / Found / Incompatible / Connecting / Ready / Error` 상태와 `Detected Path`, `Version`, `Test`, `Browse`, `Use Existing`, `Use Managed`, `Update`를 제공한다. 연결 안 된 optional P2P는 정상적인 상태이며 앱 전체 오류가 아니다.

### 19.1 저장·정책 차이

- `Settings` 변경은 전역 JSON에 저장하지만 `Current Project Folder`, 프로젝트 선택 state는 호스트 State에 저장.
- 사용자 지정 실행 파일의 자동 변경·업데이트 금지. App Managed 버전만 update manager가 관리.
- UI theme/scale 변경은 오디오 콜백 재초기화를 강제로 발생시키지 않아야 한다.
- 비밀번호/API Key/쿠키 등 비밀 값은 설정 JSON에 일반 문자열로 저장하지 않는다.
- `Restore Defaults`는 Library 삭제/History 삭제를 수행하지 않는다. Data destructive actions는 독립 화면/확인.

# 20. 보안, 권한, 저작권, 오픈소스 라이선스

### 20.1 개인정보·네트워크

`SEC-001 [MUST]` 자체 회원가입·자체 샘플 업로드·자체 텔레메트리 서버 금지(추후 명시적 opt-in 필요). API 호출 시 실제 서비스를 이용하는 데 필요한 URL·metadata만 보낸다. Source lookup 과정에서 사용자 Description/Tags를 외부 서버에 전달하지 않는다.

`SEC-002 [MUST]` URL/토큰 비밀정보, qBittorrent/slskd Key, 플랫폼 로그인 쿠키는 사용자 로그인 암호 저장소를 사용. HTTP 로그·업데이트 로그·충돌 보고서에서 secrets 삭제. Clipboard 자동 읽기 OFF. 인증 쿠키의 자동 수집/전송 기본 금지.

### 20.2 다운로드 보안

`SEC-003 [MUST]` URL scheme allowlist (`https`, 필요하면 `http`, `magnet`은 별도 처리), redirection validation, localhost/private network SSRF 위험 감소, response size/time limits, TLS 검사. HTTP 임의 명령이나 텍스트에 숨긴 `file://`/shell code 실행 금지.

`SEC-004 [MUST]` 외부 도구는 argv·proc launcher로 실행하고 shell string interpolation 사용하지 않는다. 입력 파일명, UTF-8 metadata, 코덱 인자, ffmpeg concat/subtitle 인자의 경로 injection 방어. 임시 파일 권한 제한, file symlink/traversal/Zip Slip 제한. 실행 가능한 스크립트/압축 내 프로그램 자동 실행 금지.

`SEC-005 [MUST]` helper IPC의 per-user authentication/ACL, protocol version, rate limit(필요시), cross-user session 접근 제한, 노출된 localhost API 비활성화를 적용한다. 외부 프로그램 Remote WebUI는 사용자가 켰을 때만 사용하고 보안 경고.

`SEC-006 [MUST]` 자동 업데이트는 신뢰된 upstream + checksum/signature + rollback; GitHub Action runner의 토큰 권한 최소화; 외부 artifact의 무결성 검증. OSS license 및 폰트 license를 제공.

### 20.3 사이트 약관과 권리

`SEC-007 [MUST]` 소스 사용과 ① 접근 ② 다운로드 ③ 사적 복제 ④ 샘플 사용 ⑤ 결과물 배포 권리는 독립적으로 검토될 사안임을 명시. yt-dlp 사용/재배포 가능 여부가 특정 콘텐츠의 다운로드나 저작권을 허가하는 것은 아니다.

`SEC-008 [MUST]` Spotify와 Apple Music의 DRM 보호 스트림 직접 추출/우회 금지. Spotify metadata를 이용한 YouTube 매칭·cross-service display 방식은 실제 공개 전에 플랫폼 약관을 별도로 재검토하고 정책에 따라 feature flag로 비활성화 가능해야 한다. 다운로드 불가 콘텐츠를 우회 획득한 것처럼 표현하지 않는다.

`SEC-009 [MUST]` slskd/qBittorrent 연동은 합법적으로 유통·이용할 권리를 가진 콘텐츠 대상. 트래커/검색 플러그인을 통해 저작권 침해를 기본 유도하지 않는다. 설치 시 사용자에게 명료하고 중립적인 사용권 안내.

### 20.4 소스코드/바이너리 라이선스

`SEC-010 [MUST]` **제품 자체 오픈소스 라이선스는 `OPEN-LICENSE-01`로 사용자 승인 전 미확정.** MIT가 후보일 수 있으나 별도 승인 없이 출처 명시·외부 바이너리 license와 섞지 않는다. Code License, Font License, Third Party License를 구별한다.

`SEC-011 [MUST]` yt-dlp 코드(Unlicense)와 공식 PyInstaller 실행 파일(GPLv3+ 구성요소 포함)을 구별. FFmpeg는 구성에 따라 LGPL/GPL 및 코덱별 조건 차이. iPlug2 zlib-like·Galmuri OFL·리디바탕 배포 조건·둥근모꼴 배포 허가를 별도 확인. 필요한 저작권 고지/소스 제공 의무/소스 다운로드 위치 및 변경 내역 안내.

`SEC-012 [MUST]` 프로그램과 외부 GPL 바이너리의 독립된 프로세스 실행, installer 묶음, 수정·재배포 방식은 실제 라이선스 의무 검토 전 확정하지 않는다. 오픈소스 공개만으로 모든 제3자 의무가 자동 해결된다고 주장하지 않는다.

# 21. 성능·안정성·오류 동작

### 21.1 성능 목표 (초기 수락 목표, 실기기 테스트로 조정)

`TST-001 [MUST]` 다운로드·decode·인덱스 재구축이 DAW 오디오 callback의 blocking operation이나 데드락을 발생시키지 않아야 한다. Helper crash/timeout 중에도 DAW Playback·프로젝트 저장이 정상 동작해야 한다.

`TST-002 [MUST]` 일반 UI 입력이 진행 중 다운로드 10건 또는 길이 60분 이상 waveform 분석에도 응답 가능해야 한다. 빠른 검색·메타데이터 필터는 SQL 인덱스와 pagination/virtualized list 사용. 모바일 디스플레이가 아닌 DAW 데스크톱을 우선하되 창 축소 지원.

`TST-003 [SHOULD]` 기본 목표: GUI 이벤트 체감 지연 100ms 이내(비동기 요청 자체는 별개), 일반 로컬 Preview 재생 지연 250ms 이하(지원 환경·캐시 히트 기준), 10,000 샘플 목록도 일정한 스크롤 프레임 유지. 외부 사이트 다운로드 속도는 제품 보장 대상에서 제외.

`TST-004 [MUST]` 메모리: 전체 긴 음원을 RAM에 디코딩하여 상시 유지하지 않음. peak cache multi-resolution, 오디오 read-ahead, decoded chunk cache, 라이브러리 lazy list; cache upper bound 설정.

### 21.2 공통 오류 처리 표준

각 오류는 다음 데이터를 제공한다: `errorCode`, `stage`, `sourceAdapter`, `displayMessage`, `technicalMessage`, `retryable`, `suggestedAction`, `logReference`; 사용자 화면에는 내부 파일 경로나 비밀값을 노출하지 않는다.

| 종류 | 예시 | 예상 동작 |
|---|---|---|
| 네트워크 | Offline/DNS/Timeout | 큐 대기·오프라인 Library 보존 |
| 웹 | 403/404/429/region/DRM | 다운로드 실패 이유·수동 URL/권리 관련 안내 |
| Extractor | yt-dlp 구버전·EJS/Deno 미설정 | Dependency Test/Update 안내 |
| 파일 | No space/permission/read-only | 드래프트/임시 원본 보존·새 경로 선택 |
| 오디오 | Unsupported codec/zero-length/corrupt | 오류 표시, 잘못된 파일 Library 등록 방지 |
| 변환 | ffmpeg crash/output mismatch | 원본 보존·재시도·부분 출력 격리 |
| Library | Missing file/DB corrupt/sidecar mismatch | Reconnect/Reindex/Backup, silent delete 금지 |
| qBittorrent | WebUI off/auth/seed absence | 연결 설정·외부 클라이언트 상태 표시 |
| Soulseek | Peer offline/queue/file missing | 전송 상태/재시도, 가짜 완료 금지 |
| DAW | OS file drag rejected/host unload | Floating window/copy path fallback |
| Update | Signature failed/incomplete installer | rollback, 기존 프로그램 유지 |

`TST-005 [MUST]` 모든 destructive action은 `Target + Scope + Cannot Undo`를 정확히 보여야 한다. `Clear Cache`, `Clear History`, `Remove Library Entry`, `Delete Actual Audio`, `Stop Transfer`, `Remove Torrent` 등의 의미를 섞지 않는다.

`TST-006 [MUST]` 중단·강제 종료 후 Recovery 테스트: 다운로드 중단 → 부분 파일 유지/정리, Sidecar write halfway → 재구축, update interrupted → rollback, 장치 분리 → Missing State, 같은 샘플을 두 인스턴스 동시 편집 → version conflict.

# 22. 자동화 테스트·실기기 QA

### 22.1 테스트 층위

`TST-007 [MUST]` 단위 테스트: URL parse/canonicalization, time parser, trim boundaries, metadata schema validator, Tag normalization, description storage, duplicate selection, filename sanitizer, path traversal, adapter capabilities, error code mapping.

`TST-008 [MUST]` 통합 테스트: Helper IPC version/reconnect, downloader process spawn/timeout/cancel, ffmpeg output validation, SQLite migration/reindex, sidecar DB consistency, cache eviction, mock qBittorrent/slskd HTTP, Watcher stable-file detection, installer path layout.

`TST-009 [MUST]` UI 테스트: 폰트 존재·렌더링, tab order, no system fallback in own canvas, 창 resize, DPI 100/125/150/200%, Korean IME, hover/focus, long labels/filenames, mouse wheel, disabled actions, empty states, dialogs, file drag fallback.

### 22.2 대상 플랫폼 매트릭스

| 환경 | 빌드 | Plugin 로드 | 실제 Drag | 프로젝트 State | Installer |
|---|---|---|---|---|---|
| Windows + FL Studio | 필수 | 필수 | Playlist/Sampler/Edison | 필수 | Windows Setup |
| Windows + Ableton | 필수 | 필수 | Arrangement/Session/Simpler | 필수 | Windows Setup |
| macOS + FL Studio | 필수 | 필수 | 개별 drop target | 필수 | PKG |
| macOS + Ableton | 필수 | 필수 | Arrangement/Session/Simpler | 필수 | PKG |
| macOS + Logic | 필수 | AU Validation | Tracks/Quick Sampler | 필수 | PKG |
| Windows Standalone | 필수 | N/A | 파일 관리자/DAW | 설정 복원 | Windows Setup |
| macOS Standalone | 필수 | N/A | Finder/DAW | 설정 복원 | PKG |

실제 OS/DAW가 없는 환경에서 GitHub Actions **컴파일 성공**만으로 `host loading`, `AU Validation`, `Drag 성공`까지 통과했다고 보고하지 않는다. 사용자 수동 테스트가 필요하면 정확한 클릭 경로, 스크린샷·로그 수집법을 제공.

### 22.3 필수 시나리오 QA

- **QA-WEB-01** 직접 YouTube video URL에서 지원 audio 확인 → section 다운로드 → 정확한 WAV 길이.
- **QA-WEB-02** Bandcamp album/지원 Playlist → 일부 트랙 선택 → queue 후 완료 확인.
- **QA-WEB-03** 404/403/429/없는 audio/변경된 extractor/DRM → 정확한 오류와 안전한 복구.
- **QA-WEB-04** 1초 미만 짧은 구간, 파일 끝의 구간, 장시간 1h source, 같은 URL 다른 range.
- **QA-MATCH-01** Spotify title/artist 불완전 → 수동 수정 → 후보 매칭/No match.
- **QA-MATCH-02** Apple region·track ID → lookup, 다중 후보·곡길이 불일치.
- **QA-MATCH-03** Live/remix/slowed/cover, Topic channel, 같은 이름 다른 artist 구분.
- **QA-EDT-01** stereo/mono, in/out 밀리초, zoom, loop, playback, Mono Export, fade.
- **QA-EDT-02** 16/24/float WAV, AIFF, FLAC, native rate, 비지원 조합 오류.
- **QA-P2P-01** qBittorrent API disabled, auth fail, magnet, selected-file-only, Torrent 시딩 상태 보존.
- **QA-SLSK-01** SoulseekQt watch new file, partial file 제외, slskd remote search 및 transfer 완료.
- **QA-LIB-01** 태그·Description CRUD, Sidecar만으로 DB rebuild, duplicate file names, missing sample reconnect.
- **QA-HIS-01** `history.json`가 source/tags/description 3개만 가지는지 validator 검사, audio/history 독립 삭제.
- **QA-DAW-01** FL/Ableton/Logic 각각 drag, Plugin unload/reload, project save path state.
- **QA-OS-01** Windows/macOS clean install, uninstall, offline launch, external tools absent, update rollback.
- **QA-SEC-01** path traversal/쉘 injection/invalid URL/토큰 redaction/서명 오류.
- **QA-FONT-01** 지정 번들 폰트 없을 때 CI failure, 한글/숫자/특수기호 렌더링, local original removed 후 빌드.

### 22.4 정식 배포 차단 조건

`REL-005 [MUST]` 아래 항목에 해당하는 문제를 해소하기 전에는 Stable release 금지:

1. GUI/Download 때문에 DAW audio dropout 또는 crash.
2. Library sample 파일 자동 손실·덮어쓰기, History 삭제로 파일 삭제, uninstall로 사용자 샘플 삭제.
3. 외부 qBittorrent/seeding/shared Soulseek 원본 임의 변경.
4. 정상 파일이 아닌 partial file을 WAV처럼 Library에 등록.
5. 다운로드 라이선스·DRM 제한이 있는데 성공으로 표시.
6. 인증 정보 노출·원격 API 노출·임의 명령 실행 취약점.
7. 폰트 재배포 라이선스 불명확 또는 OS UI 제외 기본 시스템 폰트 사용.
8. 지원한다고 명시한 형식의 VST/AU에 실빌드·로드 검증 부재.
9. 의존성 LICENSE/NOTICE 미비, 패키지 설치·제거 중 사용자 데이터 파괴.
10. 코드 서명/공증이 필요하다고 표기한 배포형태에서 배포 요건 미충족.

# 23. Codex 개발 계획: P00-P09

**공통 규칙:** `1 branch = 1 reviewable scope`, 작업 시작 전 master spec 확인, 작은 commit, CI 성공 확인, 해당 요구사항 ID를 PR body에 표시, self-test. 사용자 승인 없이 다음 phase 시작 금지. 독립적 단계라도 spec을 위반하는 편의 기능을 추가하지 않는다.

| 단계 | 핵심 산출물 | 종료 조건(실제 검증) |
|---|---|---|
| **P00** Bootstrap | 프로젝트 CMake/iPlug2, Standalone·VST3·AUv2 최소 UI, Helper ping, font/license inventory, CI | Windows/macOS 빌드 증거·폰트 포함·임시 UI; 다운로드 미구현 |
| **P01** Full UI Shell | 6탭·공통 component·design tokens·responsive UI·Settings structure | 화면 resize·한글·상태·탭·스크린샷 검증 |
| **P02** Audio Engine | Decoder·Editor·Preview·Waveform·IN/OUT·export | Local file에서 각 포맷/길이/Drag 준비 검증 |
| **P03** DAW | 플러그인 state, FL/Ableton/Logic drag/fallback, 프로젝트별 경로 | 실제 host 로딩/드래그/프로젝트 재실행 결과 보고 |
| **P04** Library/History | Sidecar, SQLite, Tags/Description, 3-field JSON History, Inbox | CRUD, Reindex, 장치 분리·삭제 독립성·backup 테스트 |
| **P05** Web Acquire | yt-dlp/FFmpeg/Deno/EJS 감지, SourceResolver, section download, job queue | 정상 URL·에러·캐시·재시도·출력 검증 |
| **P06** Metadata Match | Spotify/Apple public info, MusicBrainz, YouTube candidates, score | 정확/오인 후보 UI·요청 제한·권리 확인 흐름 |
| **P07** P2P | qBittorrent API, SoulseekQt Watch, slskd, P2P Inbox | 기존 클라이언트 데이터 보존·전송상태 검증 |
| **P08** Advanced & Hardening | batch/auto slice/search/filter/packaging prep/performance/recovery, optional Capture | Regression + data corruption tests |
| **P09** Install & Release | NSIS, macOS PKG, dependency setup, CI artifact, license bundle | Clean install/upgrade/uninstall, signed release 정책, stable QA |

### 23.1 리뷰 루프

1. 소유자 지시/승인 + 현재 stage 지정.
2. Codex가 `MASTER_BLUEPRINT.md`와 `AGENTS.md`를 읽고 해당 ID 목록·검증 방법을 먼저 선언.
3. Codex는 feature branch에서 코드·테스트·문서를 변경하고 커밋/PR 생성.
4. Codex 최종 보고: 변경 사항, SHA, 테스트 명령/출력, 사용한 자산의 license, 미해결 위험, 수동 QA 필요 항목.
5. Reviewer가 GitHub 파일·diff·Workflow 로그와 요구 ID를 비교해 `Critical/Major/Minor/Enhancement`로 문제 기록.
6. 수정 가능한 오류를 같은 branch/별도 review branch에 패치, 필요하면 Codex 수정 지시.
7. 필수 빌드·테스트 통과, 남은 미검증 표시 후 소유자에게 승인 요청.
8. 승인되면 main merge/기준 문서 갱신, 다음 stage.

### 23.2 버그 우선순위

- `Critical`: 데이터 소실, DAW crash/dropout, RCE/secret exposure, 라이선스 배포 차단.
- `Major`: 필수 기능 미동작, Windows/macOS 빌드 실패, 호스트 로드/Drag 실패, History 계약 위반.
- `Minor`: 대체 기능은 있으나 UI 결함, 태그 정렬, 작동하지만 불편한 edge case.
- `Enhancement`: 사용성이 좋아지는 추가 기능. 사용자 허가 없이 MVP 범위를 방해하지 않음.

### 23.3 변경관리 문서화

각 PR template에 다음 체크리스트를 넣는다:

```text
Requirement IDs:
Scope / Non-scope:
Behavioral changes:
Data migration required? (Y/N):
UI/font/palette impact:
Windows build/test:
macOS build/test:
DAW host manual QA:
Installer/License impact:
Risk and rollback:
Unresolved points / screenshots/logs:
```

Codex에게 방대한 파일을 재작성하게 하지 말고 개별 Section·Req ID를 기준으로 최소 diff를 요구. 원본 History JSON/Sidecar 스키마를 임의로 바꾸지 않으며, 마이그레이션이 필요하면 테스트와 사용자 승인 필수.

# 24. 미확정 결정과 검증이 필요한 쟁점

아래 항목을 유저가 검토·수정할 수 있도록 의도적으로 **확정하지 않는다**. Codex가 빈칸을 임의 가정해 개발하지 않도록 한다.

| ID | 미확정 항목 | 현재 권장 / 필요한 결정 |
|---|---|---|
| `OPEN-NAME-01` | **RESOLVED (P00.3)** 제품명/브랜드 | `tokkebi` (lowercase) |
| `OPEN-LICENSE-01` | 앱 코드 전체 라이선스 | MIT / GPL / 기타 중 사용자 결정 필요 |
| `OPEN-FONT-04` | 사용자가 말한 네 번째 폰트 | `C:\Users\Jung Chan\Desktop\font` 안의 실제 파일 확인 필요 |
| `OPEN-FONT-05` | 둥근모꼴·리디바탕 재배포 버전 | 정확한 원본/라이선스 파일·해시 확보 필요 |
| `OPEN-AUDIO-01` | Preview/Plugin signal handling 정책 | Passthrough 기본, Preview host audio로 별도 믹스(검증 필요) |
| `OPEN-UI-01` | 기본 창 크기/Panel 비율 | 초기 치수는 프로토타입 검증 후 조정 |
| `OPEN-UI-02` | 직접 이미지·CGI 리소스 제작 방식 | 외부 유료 자산 없이 원본 그래픽 또는 라이선스 허용 자산만 |
| `OPEN-WEB-01` | 섹션 다운로드가 네트워크를 절감하는 정도 | yt-dlp extractor·스트림별 실험 필요 |
| `OPEN-WEB-02` | YouTube 검색의 지속 가능성 | 공개 API와 extraction 안정성·서비스 정책 확인 |
| `OPEN-LEGAL-01` | Spotify/Apple metadata 기반 cross-service matching | 공개 배포 전 약관 검토; feature flag로 비활성화 가능 |
| `OPEN-DAW-01` | Plugin 내부 external drag 동작 | FL/Ableton/Logic 실제 버전별 검증 필요 |
| `OPEN-DAW-02` | 프로젝트 경로 자동 감지 | host별 API 연구, 수동 Browse는 필수 |
| `OPEN-DEP-01` | Full/Minimal Installer 기본 | Full 선호, 외부 GPL 바이너리 배포 의무 검토 필요 |
| `OPEN-INS-01` | macOS Developer ID/Notarization | 실제 배포용 계정·서명 계획 필요 |
| `OPEN-RELEASE-01` | 최초 공식 지원 DAW/OS 버전 범위 | 실제 QA 가능한 버전 기준 확정 |
| `OPEN-RESOLVE-01` | History 없는 Soulseek/Local provenance | Library sidecar에만 저장, History는 URL-only 유지 |
| `OPEN-ADV-01` | Web Capture 포함 시기 | DRM 우회 방지 및 OS capture tests 확인 후 |

## 24.1 승인·수정 방법

사용자가 수정하고 싶은 부분을 발견하면 다음처럼 기록한다:

```text
CHANGE REQUEST
Section / Requirement ID: (예: HIS-001)
Before: 기존 문구나 동작
After: 새 문구나 동작
Reason: 바꾸려는 이유
Data impact: 기존 파일/프로젝트 마이그레이션 필요 여부
Priority: Must / Should / Future / Remove
```

사용자 변경 지시가 단순한 채팅이어도 변경 사항을 문서에 반영해야 한다. Codex 작업 중 기존 요구 ID를 삭제하거나 재활용하는 대신 `Deprecated` 상태와 대체 ID를 기록해 추적성을 지킨다.

# 25. 검증된 주요 기술 참고 자료 및 원본 링크

1. iPlug2 공식 CMake/대상 형식: https://github.com/iPlug2/iPlug2/blob/master/Documentation/cmake.md
2. iPlug2 라이선스: https://github.com/iPlug2/iPlug2
3. yt-dlp README, license, extractors, `--download-sections`: https://github.com/yt-dlp/yt-dlp/blob/master/README.md
4. yt-dlp EJS & Deno: https://github.com/yt-dlp/yt-dlp/wiki/EJS
5. qBittorrent WebUI API 버전별 문서: https://github.com/qbittorrent/qBittorrent/wiki/WebUI-API
6. slskd 프로젝트: https://github.com/slskd/slskd
7. slskd 이벤트/웹훅: https://github.com/slskd/slskd/blob/master/docs/config.md
8. Spotify oEmbed: https://developer.spotify.com/documentation/embeds/reference/oembed
9. Apple iTunes Search API: https://developer.apple.com/library/archive/documentation/AudioVideo/Conceptual/iTuneSearchAPI/
10. MusicBrainz API 규칙: https://musicbrainz.org/doc/MusicBrainz_API
11. Galmuri v2.40.4: https://github.com/quiple/galmuri/releases/tag/v2.40.4
12. Galmuri license: https://github.com/quiple/galmuri/blob/main/OFL.txt
13. Noonnu 리디바탕: https://noonnu.cc/font_page/324
14. Noonnu 둥근모꼴+ (최종 원본 다시 확인): https://noonnu.cc/font_page/250
15. Apple 배포 코드 서명·공증: https://developer.apple.com/documentation/security/notarizing-macos-software-before-distribution
16. FFmpeg 라이선스: https://ffmpeg.org/legal.html

---

# 25.1 Canonical 34-Task Development Schedule

이 표는 구현 순서의 유일한 기준이다. `P00=Tasks 01–04`, `P01=05–07`, `P02=08–11`, `P03=12–14`, `P04=15–18`, `P05=19–22`, `P06=23–24`, `P07=25–28`, `P08=29–31`, `P09=32–34`이다.

| Phase | Development Task | Canonical scope |
|---|---:|---|
| P00 | 01 | Master documents / AGENTS.md / GitHub structure |
| P00 | 02 | Minimal iPlug2/CMake Standalone build |
| P00 | 03 | VST3/AUv2 targets + GitHub Actions build verification |
| P00 | 04 | Font assets / licenses / bundle verification |
| P01 | 05 | Color/font/button/panel design tokens |
| P01 | 06 | WEB/P2P/INBOX/LIBRARY/HISTORY/SETTINGS screen structure |
| P01 | 07 | Common input components, status bar, Settings, responsive window behavior |
| P02 | 08 | Local file loading and audio information |
| P02 | 09 | Playback / Stop / Volume / Audio output |
| P02 | 10 | Waveform / Zoom / range markers |
| P02 | 11 | IN/OUT extraction and WAV/etc export |
| P03 | 12 | Plugin State and project path settings |
| P03 | 13 | OS file Drag & Drop and fallback behavior |
| P03 | 14 | FL Studio / Ableton / Logic compatibility and project reopen verification |
| P04 | 15 | JSON Sidecar and SQLite schema |
| P04 | 16 | Library CRUD, Tags, Description, Search |
| P04 | 17 | text-only History |
| P04 | 18 | Inbox and separation of source/cache/final files |
| P05 | 19 | yt-dlp / FFmpeg / Deno detection and Helper execution |
| P05 | 20 | URL resolution and normal file download |
| P05 | 21 | Download queue / progress / cancel / retry |
| P05 | 22 | Section download and local cache reuse |
| P06 | 23 | Spotify / Apple Music public metadata lookup |
| P06 | 24 | YouTube candidate search / scoring / manual selection |
| P07 | 25 | qBittorrent connection and Magnet handling |
| P07 | 26 | Torrent file selection and completed-file import |
| P07 | 27 | SoulseekQt download-folder watcher |
| P07 | 28 | slskd search/download API |
| P08 | 29 | Playlist / album / batch sample processing |
| P08 | 30 | Recovery / data integrity / performance optimization |
| P08 | 31 | Auto Slice and other advanced editing + conditional Web Capture review |
| P09 | 32 | Windows NSIS install/update/uninstall |
| P09 | 33 | macOS PKG install/sign/notarization configuration |
| P09 | 34 | GitHub Releases / license notices / installation and regression QA |

**Phase exit conditions:** P00 builds the real app/base plugin targets with governing documents and fonts present. P01 provides navigable tabs with correct Korean text, fonts, graphics and supported scaling. P02 opens local audio and saves a selected interval with exact expected duration. P03 opens reliably in supported DAWs and brings samples into a DAW; untested host cases remain `NOT TESTED`. P04 stores, searches and previews samples offline while History stays independent. P05 turns supported URLs into sectionable audio imported to Library. P06 identifies intended tracks and offers source candidates, with unresolved policy integrations feature-flagged. P07 imports audio to Inbox without damaging external client state. P08 safely recovers from interrupted, missing-file and data-integrity failures under large/batch workloads. P09 lets a normal user install, use, update and remove supported packages.

## 25.2 Canonical Current Development Status

- **Development Task 08 — COMPLETE / MERGED:** PR #7 (`cfafc0b1dedfdda98c88e5fb1178de598c3ce3f7`) delivered local file loading, WAV decoding, audio information and the async loader foundation.
- **Development Task 09 — NOT COMPLETE / NEXT FUNCTIONAL TASK:** Playback, Stop, Volume and Audio output. Its eventual scoped work must absorb relevant preview transport requirements from `EDT-007`, `EDT-008` and resolved/approved `EDT-009`. It is not started by this recovery.
- **Development Task 10 — PARTIALLY IMPLEMENTED EARLY:** PR #8, merged as `8b89d509454e82ad9b4e9c552852bd3f108c81ec`, provides real waveform, multiresolution peak cache, mono/L/R, time ruler, zoom, horizontal scrolling and viewport work. PR #9 preserves additional early selection foundation but is Draft and parked. Missing work includes range markers, IN/OUT visual and numeric interaction, waveform drag selection and other frozen Task 10 interactions.
- **Development Task 11 — NOT STARTED:** IN/OUT extraction, WAV/AIFF/FLAC etc export, and exact output-duration validation.

### 25.3 Historical ordering deviation

Development Task 10 waveform/zoom foundation was implemented early in PR #8 before Development Task 09 playback. This remains valid merged work; history is not rewritten. The project returns to canonical order by completing Development Task 09 before resuming remaining Development Task 10 work. No Development Task after 09 is currently authorized for new implementation.

### 25.4 Durable workflow commitments

- A Codex implementation report is not external approval; reviewers independently inspect GitHub source, diff, tests and CI.
- Corrections remain in the same functional stage. After acceptance scope is frozen, Minor/Enhancement ideas do not indefinitely extend that stage.
- Implementation COMPLETE and CI IN_PROGRESS are separate valid states. Do not require another Codex run merely to learn whether CI finished.
- Do not self-merge a new or unreviewed functional PR. An externally approved exact head may be merged without asking for the same approval again.
- Do not end required implementation with a progress-only `INCOMPLETE` report except for a genuine external blocker.

# 26. 마스터 문서 변경 이력

| Version | Date | Status | Changes |
|---|---|---|---|
| `0.2.9-roadmap-recovery` | 2026-10-14 | **ROADMAP RESTORED / EXTERNALLY REVIEWED** | Canonical 01–34 global task plan and status were restored. Task 08 is merged; Task 09 is the next functional task; Task 10 early waveform work remains valid and PR #9 selection work is parked. No product behavior changed. |
| `0.2.9.2` | 2026-10-14 | **P02.9.2 frozen final correction authorized** | waveform combined UI state, finest-peak zoom bound, overflow-safe time decomposition, deterministic coordinator and full peak/viewport/render/ruler regression coverage만 보정한다. P02.10 및 playback/selection/export는 승인하지 않았다. |
| `0.2.9.1` | 2026-10-09 | **P02.9.1 correction authorized** | P02.9 waveform foundation의 shared PCM snapshot, 최신 generation worker, 안전한 viewport/ruler 산술, 실제 INBOX peak rendering 및 Debug/Release waveform test CI 대상을 보정한다. playback, selection, IN/OUT, export와 P02.10은 승인하지 않았다. |
| `0.2.8.4` | 2026-10-09 | **P02.8.4 correction authorized** | local-audio button의 포커스가 INBOX 밖에 남지 않도록 shell navigation 소유권과 arrow-key 정책만 보정한다. P02.9 기능은 승인하지 않았다. |
| `0.2.8.3` | 2026-10-09 | **P02.8.3 correction authorized** | native file-dialog callback의 AppShell 수명 분리, INBOX local-audio focus 순서, Failed AudioDocument의 empty-PCM 불변식과 결정론적 latest-request 검증만 보정한다. P02.9 기능은 승인하지 않았다. |
| `0.2.8.2` | 2026-10-09 | **P02.8.2 correction authorized** | display tick 기반 async completion, bounded single worker, worker exception/resource safety 및 일관된 local-open input 상태만 보정한다. P02.9 기능은 승인하지 않았다. |
| `0.2.8.1` | 2026-10-08 | **P02.8.1 correction authorized** | P02.8 로컬 로드를 worker 기반으로 보정하고 stale-result, RIFF 경계, decoded-memory 예산, 실제 메타데이터와 키보드 접근성을 검증한다. P02.9 기능은 승인하지 않았다. |
| `0.2.8` | 2026-10-08 | **P02.8 implementation authorized** | 소유자의 명시 승인에 따라 WAV 로컬 디코더, 일시적 AudioDocument, OS 파일 선택 진입과 결정론적 디코더 검증만 추가한다. AIFF/AIFC·FLAC은 인식 후 미지원으로 명시하며, waveform·preview·IN/OUT·export와 P02.9 이후는 승인하지 않았다. |
| `0.1.7` | 2026-10-07 | **P01.7 implementation authorized** | 소유자의 명시 승인에 따라 재사용 UI 구성요소, UTF-8 안전 입력 모델, 제네릭 목록, 모달, 알림·진행 상태와 비영속 SETTINGS 데모만 추가한다. P01.8 이후 기능과 `OPEN-FONT-04`는 승인되지 않았다. |
| `0.1.6` | 2026-10-07 | **P01.6 implementation authorized** | 소유자의 명시 승인에 따라 공유 6탭 UI 셸, 인스턴스별 탐색 상태, 반응형 레이아웃, 제한된 테마 전환과 사실 기반 빈 상태만 추가한다. P01.7 이후 기능과 `OPEN-FONT-04`는 승인되지 않았다. |
| `0.1.5` | 2026-10-07 | **P01.5 implementation authorized** | 소유자의 명시 승인에 따라 12색 중앙 팔레트, Light/Dark 시맨틱 토큰, 대비·타이포그래피·기하 토큰 및 제한된 테마 미리보기만 추가한다. P01.6 이후 기능과 `OPEN-FONT-04`는 승인되지 않았다. |
| `0.1.4` | 2026-10-07 | **P00.4 implementation authorized** | 소유자의 명시 승인에 따라 DungGeunMo, RIDIBatang, Galmuri v2.40.4의 라이선스 확인, 해시 매니페스트, 번들링 및 최소 렌더링 검증만 추가했다. `OPEN-FONT-04`는 확인되지 않아 유지하며 P01 기능은 승인되지 않았다. |
| `0.1.3` | 2026-10-07 | **P00.3.1 correction authorized** | 활성 문서의 제품 표기를 소문자 `tokkebi`로 정정하고, 대소문자를 구분하지 않는 텍스트 전용 브랜딩 검증을 추가한다. 기존 요구사항과 `OPEN-*` 결정은 변경하지 않는다. |
| `0.1.2` | 2026-10-06 | **P00.3 implementation authorized** | 소유자의 명시 승인에 따라 제품명을 `tokkebi`로 확정하고 저장소를 이전했다. P00.3 범위의 Standalone/VST3/AUv2 타깃, 리소스와 CI만 추가 가능하다. `OPEN-NAME-01`은 해결되었으며 나머지 `OPEN-*` 결정은 유지한다. |
| `0.1.1` | 2026-10-06 | **P00.2 implementation authorized; specification under active revision** | 소유자의 명시 승인에 따라 P00.2 native Standalone bootstrap만 시작 가능. 기존 요구사항 ID와 `OPEN-*` 결정은 유지하며 P00.3 이후는 승인되지 않음. |
| `0.1.0` | 2026-10-06 | **DRAFT, awaiting owner review** | 최초 통합 블루프린트 작성. 화면/디자인·폰트·전 소스·오디오/Library/History·DAW·Helper·설치·라이선스·QA·작업 절차 정리. 코드 개발 미착수. |

> **STOP RULE:** Development Task 09 (Playback / Stop / Volume / Audio output)가 다음 기능 작업이다. 명시적 소유자 지시 없이는 Task 09 구현을 시작하지 않는다. parked 상태인 Task 10 작업을 재개하거나 Task 11 이후 기능을 시작하지 않는다. `OPEN-AUDIO-01`은 열린 상태로 유지한다.
