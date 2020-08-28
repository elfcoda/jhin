bug:  
1. 需要加个函数标记，判断此符号是不是函数，不是要报错(fixed)
2. 如果func是个无参函数，我们使用的时候传入参数func(1)，需要报错
ExpN
|   \
func 1
3. 如果是个有参函数，我们不传参数，需要报错
AST
|  \
func
4. 如果无参函数出现在算符中，如func() + func()，应该直接求值，叶子节点
AST
|   \
func  func

5. 对return类型的判断，借助栈(fixed)  

func(1, 2, 3)
func()

