# cgroup 기반의 trace-replay

현재 이 프로그램은 [trace-replay](https://github.com/wuqiulin1995/trace-replay)를 사용하여 성능 측정을 진행합니다.

프로그램을 돌리기 위해서는 리눅스 커널에서 `CONFIG_CGROUPS`가 활성화되어 있어야 사용할 수 있습니다.
컴파일을 하기 전에 `src/main.c`에서 아래의 매크로 값을 설정해줘야 합니다.
```c
#define MAX_TASK (<number of tasks>) // 생성할 cgroup 태스크의 갯수
#define <scheduler name>_SCHEDULER // IO scheduler의 설정
#define Q_DEPTH "<IO queue depth>" // I/O queue depth 설정
#define NR_THREAD "<number of thread>" // 사용하는 thread의 수 설정
```
여기서 `<scheduler name>_SCHEDULER`에서 사용가능한 IO scheduler는 `NONE_SCHEDULER`, `BFQ_SCHEDULER`, `KYBER_SCHEDULER`의 3개가 현재는 사용 가능합니다.

먼저, 프로그램을 실행하기 위해서는 `config.cfg` 파일이 실행파일과 동일 위치에 있어야 합니다.

`config.cfg`의 구성은 아래와 같은 방식으로 해주셔야 하며, 줄의 수는 `MAX_TASK` 값을 넘겨서는 안됩니다. 좌측에 들어가는 값은 cgroup의 blkio의 io scheduler에 들어가는 weight의 값이고, 우측에 들어가는 값은 실행하고자하는 trace 파일입니다.
```bash
<weight1> <trace file directory1>
<weight2> <trace file directory2>
<weight3> <trace file directory3>
```

SNIA의 [systor '17](http://iotta.snia.org/tracetypes/3) 파일을 trace-replay 규격으로 변경하기 위해서는 [링크](https://gist.github.com/BlaCkinkGJ/2712178032fbb3366c462dfa54903fd8)의 소스 코드를 컴파일해서 실행시켜주시면 됩니다.
