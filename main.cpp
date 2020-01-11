#include <iostream>
#include <fstream>
#include <random>
#include <vector>
#include <array>
#include <random>
#include <cmath>
#include <gnuplot.hpp>
#include <stdio.h>
#include <stdlib.h>
//#include <omp.h>

/*初期条件など*/
namespace Initial_value
    {
    constexpr int N = 5000; //天体の数
    constexpr double nbody_Mass = 1.0; //N体球の質量
    constexpr double obj_mass = nbody_Mass/N; //N体球を構成する個々の天体の質量
    constexpr double radius = 1.0; //N体球の半径
    constexpr double max_speed = 0.5; //個々の天体の初期の速さの最大値
    constexpr double soft_param = radius/100.0; //ソフトニングパラメーター
    constexpr double dt = 0.01; //時間の刻み幅
    }

using namespace Initial_value;

class N_Body
{
public:
    double position[N][3];//現在のpositon
    double velocity[N][3];//現在のvelocity
    double accel[N][3];//現在のaccel
    double Mass[N];//質量
    
    N_Body()
    {
        for(auto& mass:Mass)
        {
            mass = obj_mass;
        }
    }
    
    double K_energy()
    {
        double K_E = 0;
        for(int i = 0;i<N;i++)
        {
            K_E += 0.5*Mass[i]*(velocity[i][0]*velocity[i][0]+velocity[i][1]*velocity[i][1]+velocity[i][2]*velocity[i][2]);
        }
        
        return K_E;
    }
    
    double P_Energy()
    {
        double P_E = 0;
        for(int i = 0;i<N;i++)
        {
            for(int j = 0;j<N;j++)
            {
                if(i == j)
                {
                    continue;
                }
                
                P_E += Mass[i]*Mass[j]/sqrt((position[i][0]-position[j][0])*(position[i][0]-position[j][0])+(position[i][1]-position[j][1])*(position[i][1]-position[j][1])+(position[i][2]-position[j][2])*(position[i][2]-position[j][2])+soft_param*soft_param);
            }
        }
        
        return -0.5*P_E;
    }
};

namespace Velocity_Verlet
    {
    void initial_accel(N_Body& body)
    {
        double accel[3];
        double ri[3];double dr[3];
        double ri2,mri5_2;
        double eps2 = soft_param*soft_param;
        
        for(int i = 0;i<N;i++)
        {
            for(int j = 0;j<3;j++)
            {
                accel[j] = 0;
                ri[j] = body.position[i][j];
            }
            
            for(int j = 0;j<N;j++)
            {
                if (i == j)
                {
                    continue;
                }
                
                for(int k = 0;k<3;k++)
                {
                    dr[k] = body.position[j][k]-ri[k];
                }
                
                ri2 = 1.0/(dr[0]*dr[0]+dr[1]*dr[1]+dr[2]*dr[2]+eps2);
                mri5_2 = body.Mass[j]*ri2*sqrt(ri2);
                
                for(int k = 0;k<3;k++)
                {
                    accel[k] += mri5_2*dr[k];
                }
            }
            
            for(int j = 0;j<3;j++)
            {
                body.accel[i][j] = accel[j];
            }
        }
    }
    
    void velocity_verlet(N_Body& body)
    {
        double accel[3];
        double ri[3];double dr[3];
        double ri2,mri5_2;
        double eps2 = soft_param*soft_param;
        
        for(int i = 0;i<N;i++)
        {
            for(int j = 0;j<3;j++)
            {
                body.position[i][j] += body.velocity[i][j]*dt+0.5*body.accel[i][j]*dt*dt;
            }
        }
        
        for(int i = 0;i<N;i++)
        {
            for(int j = 0;j<3;j++)
            {
                accel[j] = 0;
                ri[j] = body.position[i][j];
            }
            
            for(int j = 0;j<N;j++)
            {
                if (i == j)
                {
                    continue;
                }
                
                for(int k = 0;k<3;k++)
                {
                    dr[k] = body.position[j][k]-ri[k];
                }
                
                ri2 = 1.0/(dr[0]*dr[0]+dr[1]*dr[1]+dr[2]*dr[2]+eps2);
                mri5_2 = body.Mass[j]*ri2*sqrt(ri2);
                
                for(int k = 0;k<3;k++)
                {
                    accel[k] += mri5_2*dr[k];
                }
            }
            
            for(int j = 0;j<3;j++)
            {
                body.velocity[i][j] += 0.5*(body.accel[i][j]+accel[j])*dt;
                body.accel[i][j] = accel[j];
            }
        }
    }
    }

void set_randomValue(N_Body& body)
{
    std::random_device seed_gen;
    std::default_random_engine engine(seed_gen());
    std::uniform_real_distribution<double> dist(0.0,2*radius);
    std::uniform_real_distribution<double> dist2(0.0,2*max_speed);
    
    double r[3];
    
    for(auto& pos_row:body.position)
    {
        do
        {
            for(int i = 0;i<3;i++)
            {
                r[i] = radius - dist(engine);
            }
        }while(r[0]*r[0]+r[1]*r[1]+r[2]*r[2] > radius*radius);
        
        for(int i = 0;i<3;i++)
        {
            pos_row[i] = r[i];
        }
    }
    
    for(auto& vel_row:body.velocity)
    {
        do
        {
            for(int i = 0;i<3;i++)
            {
                r[i] = max_speed - dist2(engine);
            }
        }while(r[0]*r[0]+r[1]*r[1]+r[2]*r[2] > max_speed*max_speed);
        
        for(int i = 0;i<3;i++)
        {
            vel_row[i] = r[i];
        }
    }
}

void Initial_calibration(N_Body& body)
{
    double calibrated_val[3] = {};
    
    for(int i = 0;i<N-1;i++)
    {
        for(int j = 0;j<3;j++)
        {
            calibrated_val[j] += body.position[i][j];
        }
    }
    
    for(int j = 0;j<3;j++)
    {
        body.position[N-1][j] = calibrated_val[j];
        calibrated_val[j] = 0;
    }
    
    for(int i = 0;i<N-1;i++)
    {
        for(int j = 0;j<3;j++)
        {
            calibrated_val[j] += body.velocity[i][j];
        }
    }
    
    for(int j = 0;j<3;j++)
    {
        body.velocity[N-1][j] = calibrated_val[j];
    }
}


int main()
{
    N_Body body;
    set_randomValue(body);
    Initial_calibration(body);
    Velocity_Verlet::initial_accel(body);
    
    Gnuplot gp3;
    gp3 << "set term gif animate";
    gp3 << "set output \"n_body.gif\"";
    gp3 << "set view equal xyz";
    gp3 << "set ticslevel 0";
    
    double sphere_pos = 5.0;
    
    for(int i = 0;i<1000;i++)
    {
        std::cout << i << std::endl;
        Velocity_Verlet::velocity_verlet(body);
    }
    
    for(int i = 0;i<N-1;i++)
    {
        for(int j = 0;j<3;j++)
        {
            body.position[i][j] += sphere_pos;
        }
    }
    
    for(int i = 0;i<3;i++)
    {
        body.position[N-1][i] = 0;
    }
    
    body.Mass[N-1] = 100;
    
    for(int i = 0;i<10000;i++)
    {
        if(i%100 == 0)
        {
            gp3 << "set title \"t = "+std::to_string(dt*i)+"\"";
            gp3 << "plot[-100:100][-100:100] \"-\" with points pt 6 ps 0.5";
            for(auto& pos:body.position)
            {
                gp3 << std::to_string(pos[0])+", "+std::to_string(pos[1]);
            }
            gp3 << "e";
        }
        
        Velocity_Verlet::velocity_verlet(body);
        
        std::cout << i << std::endl;
    }
}

