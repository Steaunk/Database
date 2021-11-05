/*
 * Return Code
 */
#pragma once

enum RC { 
    SUCCESS,    // 成功不需要理由
    ERROR,      // 一般/奇怪/未命名错误
    PAGE_FULL,  // 页表已满
    ENTRY_NOT_FOUND // 记录未找到
};
