# 9cc

## What is this?

A C-language compiler made by C-language.

Following "[低レイヤを知りたい人のためのCコンパイラ作成入門](https://www.sigbus.info/compilerbook)" by [Rui Ueyama](https://github.com/rui314)

## Requiement

### Tools

* Windows 10
* Ubuntu on WSL1
* [VSCode](https://code.visualstudio.com/)

#### WSL commands

* gcc, make
  ```bash
  $ sudo apt install build-essential
  ```
* gdb
  ```bash
  $ sudo apt install gdb
  ```
* git
  ```bash
  $ sudo apt install git
  ```

#### VSCode Extensions

* [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
* [C++ Intellisense](https://marketplace.visualstudio.com/items?itemName=austin.code-gnu-global)
* [WSL workspaceFolder](https://marketplace.visualstudio.com/items?itemName=lfurzewaddock.vscode-wsl-workspacefolder)


## How to build

1. Open the repository folder by VSCode
2. `Ctrl+@` to open the terminal
3. `$ make`

## How to test

1. `Ctrl+@` to open the terminal
2. `$ make test`

You can find "OK" in a test log if all tests are done.

## How to debug

1. Open a source file you want to debug
2. Left-click at the left side of the line number you want to break
3. `Ctrl+Shift+D` to open Debug view
4. Select "9cc" configuration
5. `F5` to start debug

//