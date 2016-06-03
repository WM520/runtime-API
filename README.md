# runtime-API
### 1.遍历类的属性
```objc
// 更改类属性值
- (void)sayName
{
    unsigned int count = 0;
    Ivar *ivar = class_copyIvarList([self.person class], &count);
    
    for (int i = 0; i<count; i++) {
        Ivar var = ivar[i];
        const char *varName = ivar_getName(var);
        NSString *proname = [NSString stringWithUTF8String:varName];
        
        if ([proname isEqualToString:@"_name"]) {   //这里别忘了给属性加下划线
            object_setIvar(self.person, var, @"daming");
            break;
        }
    }
    NSLog(@"XiaoMing change name  is %@",self.person.name);
    self.textfield.text = self.person.name;
}
```

### 2.动态创建方法

```objc
- (void)sayFrom
{
    
    class_addMethod([self.person class], @selector(guess), (IMP)guessAnswer, "v@:");
    if ([self.person respondsToSelector:@selector(guess)]) {
        //Method method = class_getInstanceMethod([self.xiaoMing class], @selector(guess));
        [self.person performSelector:@selector(guess)];
        
    } else{
        NSLog(@"Sorry,I don't know");
    }
    self.textview.text = @"beijing";
}

void guessAnswer(id self,SEL _cmd){
    
    NSLog(@"i am from beijing");
    
}
```

### 3.动态交换两个方法
```objc
    Method m1 = class_getInstanceMethod([self.person class], @selector(sayName));
    Method m2 = class_getInstanceMethod([self.person class], @selector(saySex));
    
    method_exchangeImplementations(m1, m2);
```

### 4.在一个类运行的时候,动态的替换掉该类的方法
```objc
    Method m1 = class_getInstanceMethod([Person class], @selector(sayName));
    Method m2 = class_getInstanceMethod([Tool class], @selector(changeMethod));
    
    method_exchangeImplementations(m1, m2);
```

### 5.实现归档和反归档
```objc
// 字典转模型
+ (instancetype)objectWithDict:(NSDictionary *)dict
{
    // 创建对应模型对象
    id objc = [[self alloc] init];
    
    
    unsigned int count = 0;
    
    // 1.获取成员属性数组
    Ivar *ivarList = class_copyIvarList(self, &count);
    
    // 2.遍历所有的成员属性名,一个一个去字典中取出对应的value给模型属性赋值
    for (int i = 0; i < count; i++) {
        
        // 2.1 获取成员属性
        Ivar ivar = ivarList[i];
        
        // 2.2 获取成员属性名 C -> OC 字符串
        NSString *ivarName = [NSString stringWithUTF8String:ivar_getName(ivar)];
        
        // 2.3 _成员属性名 => 字典key
        NSString *key = [ivarName substringFromIndex:1];
        
        // 2.4 去字典中取出对应value给模型属性赋值
        id value = dict[key];
        
        
        // 获取成员属性类型
        NSString *ivarType = [NSString stringWithUTF8String:ivar_getTypeEncoding(ivar)];
        
        // 二级转换,字典中还有字典,也需要把对应字典转换成模型
        //
        // 判断下value,是不是字典
        if ([value isKindOfClass:[NSDictionary class]] && ![ivarType containsString:@"NS"]) { //  是字典对象,并且属性名对应类型是自定义类型
            // user User
            
            // 处理类型字符串 @\"User\" -> User
            ivarType = [ivarType stringByReplacingOccurrencesOfString:@"@" withString:@""];
            ivarType = [ivarType stringByReplacingOccurrencesOfString:@"\"" withString:@""];
            // 自定义对象,并且值是字典
            // value:user字典 -> User模型
            // 获取模型(user)类对象
            Class modalClass = NSClassFromString(ivarType);
            
            // 字典转模型
            if (modalClass) {
                // 字典转模型 user
                value = [modalClass objectWithDict:value];
            }
            
            // 字典,user
            //            NSLog(@"%@",key);
        }
        
        // 三级转换：NSArray中也是字典，把数组中的字典转换成模型.
        // 判断值是否是数组
        if ([value isKindOfClass:[NSArray class]]) {
            // 判断对应类有没有实现字典数组转模型数组的协议
            if ([self respondsToSelector:@selector(arrayContainModelClass)]) {
                
                // 转换成id类型，就能调用任何对象的方法
                id idSelf = self;
                
                // 获取数组中字典对应的模型
                NSString *type =  [idSelf arrayContainModelClass][key];
                
                // 生成模型
                Class classModel = NSClassFromString(type);
                NSMutableArray *arrM = [NSMutableArray array];
                // 遍历字典数组，生成模型数组
                for (NSDictionary *dict in value) {
                    // 字典转模型
                    id model =  [classModel objectWithDict:dict];
                    [arrM addObject:model];
                }
                
                // 把模型数组赋值给value
                value = arrM;
                
            }
        }
        
        // 2.5 KVC字典转模型
        if (value) {
            
            [objc setValue:value forKey:key];
        }
    }
    
    
    // 返回对象
    return objc;
    
}
归档
- (void)encodeWithCoder:(NSCoder *)encoder

{
    unsigned int count = 0;
    
    Ivar *ivars = class_copyIvarList([Movie class], &count);
    
    for (int i = 0; i<count; i++) {
        // 取出i位置对应的成员变量
        Ivar ivar = ivars[i];
        // 查看成员变量
        
        const char *name = ivar_getName(ivar);
        // 归档
        NSString *key = [NSString stringWithUTF8String:name];
        id value = [self valueForKey:key];
        [encoder encodeObject:value forKey:key];
        
    }
    
    free(ivars);
    
    
}
反归档
- (id)initWithCoder:(NSCoder *)decoder

{
    if (self = [super init]) {
        unsigned int count = 0;
        Ivar *ivars = class_copyIvarList([Movie class], &count);
        for (int i = 0; i<count; i++) {
            // 取出i位置对应的成员变量
            Ivar ivar = ivars[i];
            // 查看成员变量
            const char *name = ivar_getName(ivar);
            // 归档
            NSString *key = [NSString stringWithUTF8String:name];
            id value = [decoder decodeObjectForKey:key];
            // 设置到成员变量身上
            [self setValue:value forKey:key];
            
        }
        free(ivars);
        
    }
    
    return self;
    
}
```

