#include <iostream>
#include <fstream>
#include <random>
#include <vector>
#include <array>
#include <random>
#include <cmath>
#include <functional>
#include <gnuplot.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <thread>

/*初期条件など*/
namespace Initial_value
    {
    constexpr int N = 2000; //天体の数
    constexpr double nbody_Mass = 1.0; //N体球の質量
    constexpr double obj_mass = nbody_Mass/N; //N体球を構成する個々の天体の質量
    constexpr double radius = 1.0; //N体球の半径
    constexpr double max_speed = 0.5; //個々の天体の初期の速さの最大値
    constexpr double soft_param = radius/100.0; //ソフトニングパラメーター
    constexpr double dt = 0.01; //時間の刻み幅
    }

/*並列プログラム作成に必要な変数群*/
namespace thread_valiable
    {
    /*使用スレッド数*/
    constexpr int thread_count = 4; //default 1
    
    /*スレッドのtask数計算*/
    constexpr int normal_task = Initial_value::N/thread_count;
    //constexpr int extra_task = N%thread_count;
    
    /*並列計算するか否か*/
    constexpr bool do_parallel(int thread_count)
    {
        if(thread_count > 1)
        {
            return true;
        }else
        {
            return false;
        }
    }
    
    constexpr bool _do_parallel = do_parallel(thread_count);
    
    }

using namespace Initial_value;
using namespace thread_valiable;


class N_Body
{
public:
    double position[N][3];//現在のpositon
    double velocity[N][3];//現在のvelocity
    double accel[N][3];//現在のaccel
    double Mass[N];//質量
    
    N_Body()
    {
        parallel_calculation(std::bind(&N_Body::_N_Body,this,std::placeholders::_1,std::placeholders::_2));
    }
    
    void VelocityVerlet_initialStep()
    {
        parallel_calculation(std::bind(&N_Body::Ver_initial_accel,this,std::placeholders::_1,std::placeholders::_2));
    }
    
    void VelocityVerlet_calculate()
    {
        parallel_calculation(std::bind(&N_Body::update_position,this,std::placeholders::_1,std::placeholders::_2));
        
        parallel_calculation(std::bind(&N_Body::velocity_verlet,this,std::placeholders::_1,std::placeholders::_2));
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
    
    void set_randomValue()
    {
        std::random_device seed_gen;
        std::default_random_engine engine(seed_gen());
        std::uniform_real_distribution<double> dist(0.0,2*radius);
        std::uniform_real_distribution<double> dist2(0.0,2*max_speed);
        
        double r[3];
        
        for(auto& pos_row:this -> position)
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
        
        for(auto& vel_row:this -> velocity)
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

    void Initial_calibration()
    {
        double calibrated_val[3] = {};
        
        for(int i = 0;i<N-1;i++)
        {
            for(int j = 0;j<3;j++)
            {
                calibrated_val[j] += this -> position[i][j];
            }
        }
        
        for(int j = 0;j<3;j++)
        {
            this -> position[N-1][j] = calibrated_val[j];
            calibrated_val[j] = 0;
        }
        
        for(int i = 0;i<N-1;i++)
        {
            for(int j = 0;j<3;j++)
            {
                calibrated_val[j] += this -> velocity[i][j];
            }
        }
        
        for(int j = 0;j<3;j++)
        {
            this -> velocity[N-1][j] = calibrated_val[j];
        }
    }
    
private:
    void parallel_calculation(const std::function<void(int,int)>& func)
    {
        if(_do_parallel)
        {
            int thread_count_1 = thread_count - 1;
            std::vector<std::thread> threads;
            int normal_task_ = normal_task;
            for(int i = 0;i<thread_count_1;i++)
            {
                threads.emplace_back([func,i,normal_task_](){func(i*normal_task_,(i+1)*normal_task_);});
            }
            
            func(thread_count_1*normal_task_,N);
                
            for(auto& thread:threads)
            {
                thread.join();
            }
            
            }else
            {
                func(0,N);
            }
        }
    
    void _N_Body(int _i,int j)
    {
        for(int i = _i;i<j;i++)
        {
            this -> Mass[i] = obj_mass;
        }
    }
    
    void Ver_initial_accel(int _i,int end)
    {
        double accel[3];
        double ri[3];double dr[3];
        double ri2,mri5_2;
        double eps2 = soft_param*soft_param;
        
        for(int i = _i;i<end;i++)
        {
            for(int j = 0;j<3;j++)
            {
                accel[j] = 0;
                ri[j] = this -> position[i][j];
            }
            
            for(int j = 0;j<N;j++)
            {
                if (i == j)
                {
                    continue;
                }
                
                for(int k = 0;k<3;k++)
                {
                    dr[k] = this -> position[j][k]-ri[k];
                }
                
                ri2 = 1.0/(dr[0]*dr[0]+dr[1]*dr[1]+dr[2]*dr[2]+eps2);
                mri5_2 = this -> Mass[j]*ri2*sqrt(ri2);
                
                for(int k = 0;k<3;k++)
                {
                    accel[k] += mri5_2*dr[k];
                }
            }
            
            for(int j = 0;j<3;j++)
            {
                this -> accel[i][j] = accel[j];
            }
        }
    }
    
    void update_position(int _i,int end)
    {
        for(int i = _i;i<end;i++)
        {
            for(int j = 0;j<3;j++)
            {
                this -> position[i][j] += this -> velocity[i][j]*dt+0.5*this -> accel[i][j]*dt*dt;
            }
        }
    }
    
    void velocity_verlet(int _i,int end)
    {
        double accel[3];
            double ri[3];double dr[3];
            double ri2,mri5_2;
            double eps2 = soft_param*soft_param;
            
            for(int i = _i;i<end;i++)
            {
                for(int j = 0;j<3;j++)
                {
                    accel[j] = 0;
                    ri[j] = this -> position[i][j];
                }
                
                for(int j = 0;j<N;j++)
                {
                    if (i == j)
                    {
                        continue;
                    }
                    
                    for(int k = 0;k<3;k++)
                    {
                        dr[k] = this -> position[j][k]-ri[k];
                    }
                    
                    ri2 = 1.0/(dr[0]*dr[0]+dr[1]*dr[1]+dr[2]*dr[2]+eps2);
                    mri5_2 = this -> Mass[j]*ri2*sqrt(ri2);
                    
                    for(int k = 0;k<3;k++)
                    {
                        accel[k] += mri5_2*dr[k];
                    }
                }
                
                for(int j = 0;j<3;j++)
                {
                    this -> velocity[i][j] += 0.5*(this -> accel[i][j]+accel[j])*dt;
                    this -> accel[i][j] = accel[j];
                }
            }
        }
};

int main()
{
    N_Body body;
    body.set_randomValue();
    body.Initial_calibration();
    body.VelocityVerlet_initialStep();
    
    Gnuplot gp3;
    gp3 << "set term gif animate";
    gp3 << "set output \"n_body_N0_dt0.01_pos30.gif\"";
    gp3 << "set view equal xyz";
    gp3 << "set ticslevel 0";
    
    double sphere_pos = 30.0;
    
    for(int i = 0;i<3000;i++)
    {
        std::cout << i << std::endl;
        body.VelocityVerlet_calculate();
        std::cout << body.P_Energy() +  body.K_energy() << std::endl;
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
        body.velocity[N-1][i] = 0;
    }
    
    body.Mass[N-1] = 100;
    
    for(int i = 0;i<100000;i++)
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
        
        body.VelocityVerlet_calculate();
        
        std::cout << i << std::endl;
    }
}
