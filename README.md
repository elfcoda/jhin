<a href="http://flowerdance.me/"><img alt="Home" src="https://img.shields.io/badge/Home-🌏-9cf"></a>
<a href="https://www.patreon.com/wenjielu"><img alt="Support Me" src="https://img.shields.io/badge/Support%20Me-%F0%9F%92%97-green"></a>
<img alt="C++17" src="https://img.shields.io/badge/Language-C%2B%2B17-orange">
<img alt="Python3" src="https://img.shields.io/badge/Language-Python3-blueviolet">
<img alt="Version" src="https://img.shields.io/badge/Version-1.0-ff69b4">
<img alt="License" src="https://img.shields.io/badge/License-Apache--2.0-blue">  
<a href="https://llvm.org/"><img alt="Dependency" src="https://img.shields.io/badge/Dependency-LLVM-C67DEA"></a>
<img alt="Status" src="https://img.shields.io/badge/Status-🍇-15AC96">


# Jhin-lang  
  
 ## [Installation 🥧](#anc_0)  
 ## [Tutorial 🥙](#anc_1)
<h3>What is Jhin?</h3>  
"烬" in Chinese, from the game League of Legends  
Jhin is a simple Strong Static typed programming language  
  
<h3>Jhin Source Code: example.jn</h3>  
-- This is a comment
`def functionName(num: Int <- 1, pi: Float <- 3.14): Int -- function definition
{
    count: Int <- 7
    while (count <= 10)
    {
        printf(count)   -- link to C lib
        count <- count + 1
    }

    if (3.14 == pi)
    {
        return num
    }

    count
}

def main(): Int
{
    functionName(12, 2.18)
    0
}`
  
<h3>Run in compiler:</h3>  
`./jhin example.jn`  
`./output`  
  
<h3>Run in interpreter:</h3>  
`./jhini example.jn`  
  
<h3>Run in REPL:</h3>  
Waiting for the code  

 ## [Architecture 🥗](#anc_2)
 ## [Contribute 🍖](#anc_3)
 
  
<h2 name = "anc_0">Installation 🐇</h2>
<h2 name = "anc_1">Tutorial 🐿</h2>
<h2 name = "anc_2">Architecture 🐢</h2>
<img alt="Arch" src="https://github.com/elfcoda/jhin/blob/master/pic/arch.png">
<h2 name = "anc_3">Contribute 🦘</h2>

Project built in C++17    
dependency：  
- `Python3`  
- `matplotlib`  
  
How to deploy：  
1. [Building Clang and Working with the Code][0]  
2. go to llvm-project/llvm/examples/Kaleidoscope and add `add_subdirectory(Jhin)` to CMakeLists.txt    
3. `git submodule add https://<token>@github.com/elfcoda/jhin.git Jhin`   
    You should create a personal access `<token>` to use in place of a password with the command line or with the API.  
    [how to get your token][1]  
  
  
  
  
  
  
  
Now you are starving right?  
  
[0]: https://clang.llvm.org/get_started.html
[1]: https://docs.github.com/en/github/authenticating-to-github/keeping-your-account-and-data-secure/creating-a-personal-access-token  


