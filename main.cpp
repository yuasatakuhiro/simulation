#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <random>
#include <cmath>
#include <gnuplot.hpp>

/*初期条件など*/
namespace Initial_value
    {
    constexpr int obj_num = 10000;
    constexpr double nbody_Mass = 1.0;
    constexpr double obj_mass = nbody_Mass/obj_num;
    constexpr double radius = 1.0;
    constexpr double max_speed = 0.5;
    constexpr double soft_param = radius/100.0;
    constexpr double dt = 0.001;
    }

using namespace Initial_value;

/*粒体1個のオブジェクト*/
class Object
{
public:
    
    std::array<double,3> position{0,0,0}; //現在のposition
    std::array<double,3> velocity{0,0,0}; //現在からdt/2秒後のvelocity
    
    /*計算用の格納庫*/
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
    
    double length(double x0,double x1,double x2)
    {
        return sqrt((position[0]-x0)*(position[0]-x0)+(position[1]-x1)*(position[1]-x1)+(position[2]-x2)*(position[2]-x2));
    }
    
};

/*N体のオブジェクトの定義**/
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
    
    std::vector<double> density_distribution(double dr)
    {
        double r = 0;
        double r_dr;
        int count = 0;
        int num = (radius+0.3)/dr;
        
        std::vector<double> density;
        
        for(int i = 1;i<=num;i++)
        {
            r_dr = r;
            r = i*dr;
            count = 0;
            for(auto& obj1:obj)
            {
                double length_0 = obj1.length(0,0,0);
                if(r_dr <= length_0 && length_0 < r)
                {
                    count += 1;
                }
            }
            density.emplace_back(obj_mass*count/(1.33333*M_PI*(r*r*r-(r_dr)*(r_dr)*(r_dr))));
        }
        
        return density;
    }
    
};

inline std::ostream& operator<<(std::ostream& os,const Object& o)
{
    return os << o.velocity[0] << ' ' << o.velocity[1] << ' ' << o.velocity[2];
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
        obj.next_vel = obj.velocity;
    }
}


/*リープフロッグ法の計算に必要な関数群**/
namespace leap_frog
    {
    double accel[3];
    double position[3];
    double velocity[3];
    double length;
    int count;
    
    void _update_vel(n_body& body_1)
    {
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
        
    }
    
    void leap_frog(n_body& body_1)
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
    }
    }

/*初期位置と初期速度の較正（重心位置を球の中心に、重心速度を０にする）*/
namespace caliblation {
void calibrate_pos_vel(n_body& body_1)
{
    std::array<double,6> cent_grav_val;
    cent_grav_val.fill(0);
    
    for(auto obj = body_1.obj.begin();obj != body_1.obj.end()-1;obj++)
    {
        cent_grav_val[0] += obj->position[0];
        cent_grav_val[1] += obj->position[1];
        cent_grav_val[2] += obj->position[2];
        
        cent_grav_val[3] += obj->velocity[0];
        cent_grav_val[4] += obj->velocity[1];
        cent_grav_val[5] += obj->velocity[2];
    }
    
    for(int i = 0;i<3;i++)
    {
        body_1.obj[obj_num-1].position[i] = -cent_grav_val[i];
        body_1.obj[obj_num-1].velocity[i] = -cent_grav_val[i+3];
    }
    
    body_1.obj[obj_num-1].next_vel = body_1.obj[obj_num-1].velocity;
    
    
    
}
}

int main()
{
    n_body body_1;
    Gnuplot gp;
    gp << "set term gif animate";
    gp << "set output \"density_dist.gif\"";
    gp << "set xlabel \"radius\"";
    gp << "set ylabel \"density\"";
    
    Gnuplot gp2;
    gp2 << "set terminal png";
    gp2 << "set output \"e_data.png\"";
    gp2 << "set xlabel \"time\"";
    gp2 << "set ylabel \"(E0-E(t))/E(0)*100\"";
    gp2 << "set title \"Energy ratio\"";
    
    Gnuplot gp3;
    gp3 << "set term gif animate";
    gp3 << "set output \"n_body.gif\"";
    gp3 << "set view equal xyz";
    gp3 << "set ticslevel 0";
    
    Gnuplot gp4;
    gp4 << "set terminal png";
    gp4 << "set output \"energy.png\"";
    gp4 << "set xlabel \"time\"";
    gp4 << "set ylabel \"energy\"";
    
    
    set_randomval(body_1);
    
    caliblation::calibrate_pos_vel(body_1);
    
    std::vector<double> a = body_1.density_distribution(0.01);
    gp << "set title \"t = 0\"";
    gp << "plot[0:1.2][0:140] \"-\" with lines title \"density\"";
    for(int i = 0;i<a.size();i++)
    {
        gp << std::to_string(i*0.01) +", "+std::to_string(a[i]);
    }
    gp << "e";
    
    double K = body_1.K_enrgy();double U = body_1.P_enrgy();
    double berial = K/(-U);
    double E0 = K+U; //初期のエネルギー
    
    gp2 << "plot[0:] \"-\" with lines";
    gp2 << "0, 0";
    
    gp3 << "set key title \"K/|U| = "+std::to_string(berial)+"\"";
    gp3 << "set title \"t = 0\"";
    gp3 << "splot[-1.6:1.6][-1.6:1.6][-1.6:1.6] \"-\" with points pt 6 ps 0.5";
    for(auto& obj1:body_1.obj)
    {
        gp3 << std::to_string(obj1.position[0])+", "+std::to_string(obj1.position[1])+", "+std::to_string(obj1.position[2]);
    }
    gp3 << "e";
    
    gp4 << "plot[0:] \"-\" with points";
    gp4 << "0, "+std::to_string(K);gp4 << "0, "+std::to_string(U);gp4 << "0, "+std::to_string(E0);
    
    leap_frog::_update_vel(body_1);//速度をdt/2の時間分更新する
    int count = 0;
    for(int i = 0;i<2000;i++)
    {
        std::cout << i << std::endl;
        count++;
        leap_frog::leap_frog(body_1);
        if (count == 10)
        {
            K = body_1.K_enrgy();U = body_1.P_enrgy();
            berial = K/(-U);
            double E_t = K+U;
            std::string t = std::to_string(dt*(i+1));
            
            gp << "set title \"t = "+t+"\"";
            gp3 << "set title \"t = "+t+"\"";
            gp3 << "set key title \"K/|U| = "+std::to_string(berial)+"\"";
            
            gp << "plot[0:1.2][0:140] \"-\" with lines title \"density\"";
            std::vector<double> a = body_1.density_distribution(0.01);
            for(int J = 0;J<a.size();J++)
            {
                gp << std::to_string(J*0.01) +", "+std::to_string(a[J]);
            }
            gp << "e";
            
            gp2 << std::to_string(dt*(i+1))+", "+std::to_string((E0-E_t)/E0*100);
            
            gp3 << "set key title \"K/|U| = "+std::to_string(berial)+"\"";
            gp3 << "set title \"t = "+std::to_string(dt*(i+1))+"\"";
            gp3 << "splot[-1.6:1.6][-1.6:1.6][-1.6:1.6] \"-\" with points pt 6 ps 0.5";
            for(auto& obj1:body_1.obj)
            {
                gp3 << std::to_string(obj1.position[0])+", "+std::to_string(obj1.position[1])+", "+std::to_string(obj1.position[2]);
            }
            gp3 << "e";
            
            gp4 << t+", "+std::to_string(K);gp4 << t+", "+std::to_string(U);gp4 << t+", "+std::to_string(E_t);
            count = 0;
        }
    }
    
    gp2 << "e";
    gp4 << "e";
    
    
    
    
    
    
    /*of1.close();of2.close();
     
     FILE* gp;
     std::string kt;
     
     std::ifstream ifile("KT.txt");
     gp = popen("/Applications/gnuplot.app/gnuplot","w");
     
     fprintf(gp,"set term gif animate\n");
     fprintf(gp,"set view equal xyz\n");
     fprintf(gp,"set output \"n_body4:.gif\" \n");
     for(int i = 0;i<2000;i++)
     {
     getline(ifile,kt);
     //fprintf(gp,"splot [-1.2:1.2][-1.2:1.2][-1.2:1.2] -3 -3 -3 \n");
     fprintf(gp,"set key title \"K/T = %s\" \n",kt.c_str());
     fprintf(gp,"set title \"%f\"\n",10*dt*i);
     fprintf(gp,"splot  [-1.6:1.6][-1.6:1.6][-1.6:1.6] \"e_data.dat\" index %d using 1:2:3 with points pt 6 ps 0.5\n",i);
     }
     
     ifile.close();
     
     Gnuplot gp2;
     gp2 << "set terminal png";
     gp2 << "set output \"aaa.jpeg\"";
     gp2 << "plot sin(x)";*/
    
}
