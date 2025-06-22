"General settings
set incsearch
set ignorecase
set scrolloff=2
set smartcase
set wildmode=longest,list
set pastetoggle=<F2>

set expandtab
set tabstop=4
set shiftwidth=4
set autoindent
set smartindent
syntax on

set viminfo=""


" Jump to last position when reopening a file
if has("autocmd")
  au BufReadPost * if line("'\"") > 1 && line("'\"") <= line("$") | exe "normal! g'\"" | endif
endif
