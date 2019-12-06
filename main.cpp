#include <iostream>
#include <vector>
#include <array>
#include <sprout/random.hpp>

namespace Initial_value
{
constexpr int obj_num = 500;
constexpr float nbody_Mass = 1.0;
constexpr float obj_mass = nbody_Mass/obj_num;
}

using namespace Initial_value;

class Object
{
public:
    std::array<double,3> position;
    std::array<double,3> velocity;
    double mass;
    
    constexpr Object(float _mass = obj_mass):position({0,0,0}),velocity({0,0,0}),mass(_mass) {}
    
    constexpr void set_position(double x,double y,double z)
    {
        position[0] = x;
        position[1] = y;
        position[2] = z;
    }
    
    constexpr void set_velocity(double vx,double vy,double vz)
    {
        velocity[0] = vx;
        velocity[1] = vy;
        velocity[2] = vz;
    }
    
    constexpr void set_mass(double _mass)
    {
        mass = _mass;
    }
};

class n_body
{
public:
    std::vector<Object> obj();
    double Mass;
    
    constexpr n_body(float _Mass = nbody_Mass):Mass(_Mass) {}
    
    constexpr void set_position(double x,double y,double z)
    {
        
    }
};

inline std::ostream& operator<<(std::ostream& os,const Object& o)
{
    return os << o.position[0] << ',' << o.position[1] << ',' << o.position[2];
}

int main()
{
    std::cout << "rgaref" << std::endl;
}
