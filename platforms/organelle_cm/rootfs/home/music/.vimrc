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

set viminfo='100,<50,s10,h,f1

" Jump to last position when reopening a file
au BufReadPost * if line("'\"") > 1 && line("'\"") <= line("$") | exe "normal! g'\"" | endif
