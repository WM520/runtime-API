//
//  Person.h
//  runtime
//
//  Created by qianjianeng on 16/4/16.
//  Copyright © 2016年 SF. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface Person : NSObject

@property(nonatomic,copy) NSString *name;
@property(nonatomic,copy) NSString *sex;
@property(nonatomic,assign) NSUInteger age;
-(NSString *)sayName;
-(NSString *)saySex;
@end
