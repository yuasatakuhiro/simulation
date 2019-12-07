#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <random>
#include <cmath>

namespace Initial_value
{
constexpr int obj_num = 1000;
constexpr double nbody_Mass = 1.0;
constexpr double obj_mass = nbody_Mass/obj_num;
constexpr double radius = 1.0;
constexpr double max_speed = 0.5;
constexpr double soft_param = radius/100.0;
constexpr double dt = 0.001;
}

using namespace Initial_value;

class Object
{
public:
    std::array<double,3> position{0,0,0};
    std::array<double,3> velocity{0,0,0};
    std::array<double,3> next_pos{0,0,0};
    std::array<double,3> next_vel{0,0,0};
    
    Object(){}
    
    void set_position(double x,double y,double z)
    {
        position[0] = x;
        position[1] = y;
        position[2] = z;

    }
    
    void set_velocity(double vx,double vy,double vz)
    {
        velocity[0] = vx;
        velocity[1] = vy;
        velocity[2] = vz;
    }
    
    void update_next_pos(double x,double y,double z)
    {
        next_pos[0] = x;
        next_pos[1] = y;
        next_pos[2] = z;
    }
    
    void update_position()
    {
        position = next_pos;
    }
    
    double K_energy()
    {
        double _velocity[3] = {(velocity[0]+next_vel[0])/2.0,(velocity[1]+next_vel[1])/2.0,(velocity[2]+next_vel[2])/2.0};
        return 0.5*obj_mass*(_velocity[0]*_velocity[0]+_velocity[1]*_velocity[1]+_velocity[2]*_velocity[2]);
    }
    
};

class n_body
{
public:
    std::array<Object,obj_num> obj;
    double Mass;
    
    n_body(double _Mass = nbody_Mass):Mass(_Mass) {}
    
    double P_enrgy()
    {
        double E = 0;
        
        for(auto obj1 = obj.begin();obj1 != obj.end();obj1++)
        {
            for(auto obj2 = obj.begin();obj2 != obj.end();obj2++)
            {
                if(obj1==obj2)
                {
                    continue;
                }
                
                E -= obj_mass*obj_mass/sqrt((obj2->position[0]-obj1->position[0])*(obj2->position[0]-obj1->position[0])+(obj2->position[1]-obj1->position[1])*(obj2->position[1]-obj1->position[1])+(obj2->position[2]-obj1->position[2])*(obj2->position[2]-obj1->position[2]));
            }
        }
        
        return 0.5*E;
    }
    
    double K_enrgy()
    {
        double E = 0;
        
        for(auto& obj_:obj)
        {
            E += obj_.K_energy();
        }
        
        return E;
    }
};

inline std::ostream& operator<<(std::ostream& os,const Object& o)
{
    return os << o.velocity[0] << ',' << o.velocity[1] << ',' << o.velocity[2];
}

void set_randomval(n_body& body)
{
    srand((unsigned int)time(NULL));
    for(auto& obj:body.obj)
    {
        double value[3];
        do
        {
            for (int i=0;i<3;i++)
            {
                value[i] = radius-2*radius*(double)rand()/(1.0+RAND_MAX);
            }
        }
        while(value[0]*value[0]+value[1]*value[1]+value[2]*value[2] >= radius*radius);
        
        obj.set_position(value[0], value[1], value[2]);
        
        do
        {
            for(int i=0;i<3;i++)
            {
                value[i] = max_speed-2*max_speed*(double)rand()/(1.0+RAND_MAX);
            }
        }
        while(value[0]*value[0]+value[1]*value[1]+value[2]*value[2] >= max_speed*max_speed);
        
        obj.set_velocity(value[0], value[1], value[2]);
    }
}

n_body body_1;

int main()
{
    set_randomval(body_1);
    
    double accel[3];
    double position[3];
    double velocity[3];
    double length;
    
    for(auto obj1 = body_1.obj.begin();obj1 != body_1.obj.end();obj1++)
    {
        for(int i=0;i<3;i++)
        {
            accel[i] = 0;
            position[i] = obj1->position[i];
            velocity[i] = obj1->velocity[i];
        }
        
        for(auto obj2 = body_1.obj.begin();obj2 != body_1.obj.end();obj2++)
        {
            double position2[3] = {obj2->position[0],obj2->position[1],obj2->position[2]};
            
            if(obj1 == obj2)
            {
                continue;
            }
            
            length = 1.0/pow(((position[0]-position2[0])*(position[0]-position2[0])+(position[1]-position2[1])*(position[1]-position2[1])+(position[2]-position2[2])*(position[2]-position2[2]) + soft_param*soft_param),1.5);
            
            for(int i=0;i<3;i++)
            {
                accel[i] += obj_mass*(position2[i] - position[i])*length;
            }
        }
        
        for(int i=0;i<3;i++)
        {
            velocity[i] += accel[i]*dt*0.5;
        }
        
        obj1->set_velocity(velocity[0], velocity[1], velocity[2]);
        
    }
    
    for (int _ = 0;_<4000;_++)
    {
        for(auto obj1 = body_1.obj.begin();obj1 != body_1.obj.end();obj1++)
        {
            for(int j=0;j<3;j++)
            {
                accel[j] = 0;
                position[j] = obj1->position[j];
                velocity[j] = obj1->velocity[j];
            }
        
            for(auto obj2 = body_1.obj.begin();obj2 != body_1.obj.end();obj2++)
            {
                double position2[3] = {obj2->position[0],obj2->position[1],obj2->position[2]};
            
                if(obj1 == obj2)
                {
                    continue;
                }
                
                length = 1.0/pow(((position[0]-position2[0])*(position[0]-position2[0])+(position[1]-position2[1])*(position[1]-position2[1])+(position[2]-position2[2])*(position[2]-position2[2]) + soft_param*soft_param),1.5);

                for(int j=0;j<3;j++)
                {
                    accel[j] += obj_mass*(position2[j] - position[j])*length;
                }
            }
        
            for(int j=0;j<3;j++)
            {
                obj1->next_vel[j] = velocity[j];
                velocity[j] += accel[j]*dt;
                position[j] += velocity[j]*dt;
            }
        
            obj1->update_next_pos(position[0], position[1], position[2]);
            obj1->set_velocity(velocity[0], velocity[1], velocity[2]);
        
        }
        
        for(auto& obj:body_1.obj)
        {
            obj.update_position();
        }
        
        std::cout << body_1.K_enrgy()/body_1.P_enrgy() << std::endl;
    }

    
}
