let g:project_root = expand('<sfile>:p:h')
" let g:vim9llvm_target = g:project_root . '/build/app'
packadd! vim9llvm

let s:help_return_win = 0

function! s:ReturnFromHelp(timer_id) abort
    if s:help_return_win > 0 && win_id2win(s:help_return_win) > 0
        if mode() ==# 't'
            call feedkeys("\<C-w>:call win_gotoid(" . s:help_return_win . ")\<CR>", 'n')
        else
            call win_gotoid(s:help_return_win)
        endif
    endif
    let s:help_return_win = 0
endfunction

function! ShowHelp() abort
    let l:was_open = (bufwinnr('__My_Custom_Help__') != -1)
    if !l:was_open
        let s:help_return_win = win_getid()
    endif

    call ShowHelpBase()

    if l:was_open
        call timer_start(0, function('s:ReturnFromHelp'))
        return
    endif

    augroup MyCustomHelpReturn
        autocmd! * <buffer>
        autocmd BufWinLeave <buffer> call timer_start(0, function('s:ReturnFromHelp'))
    augroup END

    setlocal modifiable

    let l:extra_lines = [
        \ '',
        \ '=================== КОДИНГ И LSP (VIM9LLVM) ===================',
        \ ' F5                - Собрать проект (CMake / Ninja / Makefile)',
        \ ' Space + gd / gD   - Перейти к определению / объявлению',
        \ ' Space + gt / gi   - Перейти к типу / реализации',
        \ ' Space + gr        - Список ссылок на символ',
        \ ' K                 - Справка по символу (LSP hover)',
        \ ' Space + rn / ca   - Переименовать символ / Code action',
        \ ' Space + cf        - Форматировать файл (.clang-format)',
        \ ' Space + ld        - Список диагностик clangd',
        \ ' ]g / [g           - Следующая / предыдущая диагностика',
        \ ' Space + o / ws    - Символы файла / поиск по проекту',
        \ '',
        \ '=================== ОТЛАДЧИК LLDB (VIM9LLVM) ==================',
        \ ' F6                - Включить / выключить режим отладки',
        \ ' F7 / F8 / F9      - Continue / Next / Step In',
        \ ' F10 / F11         - Точка останова / Finish (выход из функции)',
        \ ' Space + rc        - Run to Cursor (выполнить до курсора)',
        \ ' Space + k / d     - Hover (значение в LLDB) / Disasm',
        \ ' Ctrl + g, 1..8    - Переключение окон отладчика',
        \ '===============================================================',
        \ ]

    call append(line('$'), l:extra_lines)
    setlocal nomodifiable
    normal! gg
endfunction
