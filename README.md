# Event-Driven Framework

This repository provides a framework designed to simplify the development of programs for microcontrollers (MCUs), It can run on most MCUs because it is not dependent on the architecture. System operations are driven by events that may occur during execution. For instance, when an MCU receives data via UART, rather than processing it directly within an interrupt handler—where code execution must be kept brief—one can simply call `ed_post("task_id", "signal")`. The received data will then be processed within a function associated with the specified "task\_id" and "signal." This framework is suitable only for small to medium-scale systems, as it does not guarantee 100% real-time performance. 

Key highlights:
- Low code size
- Modularized code, facilitating maintenance or the development of new features
- High portability

> [!WARNING]
> This framework is not endorsed or guaranteed for reliability by any reputable organization; please monitor your system if you choose to use it. If you encounter any errors, please report them to us.