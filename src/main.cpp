#include <iostream>
#include "pricing/pricing.h"

int main(){
    pricing::BSParams params = {100.0, 100.0, 0.05, 0.2, 1.0};
    
    double call_price = pricing::bs_call(params);
    double put_price = pricing::bs_put(params);

    std::cout << "Call price " << call_price << std::endl;
    std::cout << "Put price " << put_price << std::endl;


    return 0;
     
}