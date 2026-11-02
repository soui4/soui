/*
 * ScintillaNoExcept.h -- 让 Scintilla 在 -fno-exceptions 下仍然能编译。
 *
 * 为什么要这么做：wasm 模块里所有东西共用一个运行时的 EH 机制，Emscripten 默认
 * 就把 catch 全部编译成不可达（-sDISABLE_EXCEPTION_CATCHING=1），要真正拿到
 * try/catch 的语义只能全模块打开 -fexceptions，代价是整套 libc++abi 异常表。
 * Scintilla 的异常只用于两类场景：
 *
 *   1. 内部不变量检查（RunStyles::Check、CellBuffer::PerformUndoStep、
 *      SplitVector::ReAllocate）——只在数据已经损坏时才抛；
 *   2. 资源/正则失败的回退（Document::AddData 的 bad_alloc、BuiltinRegex 的
 *      RegexError），catch 分支都只是返回一个状态码。
 *
 * 这两类都不构成"必须展开栈"的理由，所以这里把语法降级成普通语句：
 *
 *   try { A } catch (X) { B } catch (...) { C }
 *       ==>  if (true) { A } if (false) { B } if (false) { C }
 *   throw EXPR;
 *       ==>  ::abort(), (void) EXPR;
 *
 * throw 映射到 abort()（不是静默忽略）：语义与 Emscripten 默认的"throw 即终止"
 * 一致，只是不再需要异常表。catch 分支仍然参与编译（-Wall 下仍在做类型检查），
 * 只是永远不进入。
 *
 * 只在 wasm 构建里通过 target_compile_options 的 -include 注入；桌面构建不受影响，
 * 仍按各平台默认的异常设置编译。
 */
#ifndef SCINTILLA_NO_EXCEPT_H
#define SCINTILLA_NO_EXCEPT_H

#include <cstdlib>

#define try if (true)
#define catch(...) if (false)
#define throw ::abort(), (void)

#endif // SCINTILLA_NO_EXCEPT_H
