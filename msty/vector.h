#ifndef m_vector_h_
#define m_vector_h_
#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>
#include <cstdio>
namespace msty
{
template <typename type> class Vec2
{
    /* Completed Functions:
     Addition: vector

     */
public:
    type x,y;
    Vec2()   //D
    {
        x = 0;
        y = 0;
    };
    Vec2(type x1,type y1)   //D
    {
        x = x1;
        y = y1;
    };
    Vec2(const Vec2& nv)   //D
    {
        x = (type)nv.x;
        y = (type)nv.y;
    };
    Vec2 operator + (Vec2 add)   //D
    {
        return Vec2((type)(x+add.x),(type)(y+add.y));
    };
    Vec2 operator - (Vec2 sub)   //D
    {
        return Vec2((type)(x-sub.x),(type)(y-sub.y));
    };
    Vec2 operator * (type mul)   //D
    {
        return Vec2((type)(x*mul),(type)(y*mul));
    };
    Vec2 operator / (type div)   //D
    {
        return Vec2((type)(x/div.x),(type)(y/div.y));
    };
    Vec2 operator += (Vec2 add)   //D
    {
        x += (type)add.x;
        y += (type)add.y;
        return *this;
    };
    Vec2 operator = (Vec2 assign)   //D
    {
        x = (type)assign.x;
        y = (type)assign.y;
        return *this;
    };
    Vec2 operator -= (Vec2 sub)   //D
    {
        x -= (type)sub.x;
        y -= (type)sub.y;
        return *this;
    };
    Vec2 operator *= (type mul)   //D
    {
        x *= mul;
        y *= mul;
        return *this;
    };
    Vec2 operator /= (type div)   //D
    {
        x /= div;
        y /= div;
        return *this;
    };
    Vec2<type> Lerp(Vec2<type> goal,float alpha)
    {
        return Vec2<type>((type)(this->x + alpha * (goal.x - this->x)),(type)(this->y + alpha * (goal.y - this->y)));
    }
    void Lerp2(Vec2<type> goal,float alpha)
    {
        this->x += (type)(alpha * (goal.x - this->x));
        this->y += (type)(alpha * (goal.y - this->y));
    }
    Vec2<type> Lerp3(Vec2<type> goal,type dist)
    {
        return Vec2<type>(
                   (type)(this->x + dist/this->Dist(goal) * (goal.x - this->x))
                   ,
                   (type)(this->y + dist/this->Dist(goal) * (goal.y - this->y))
               );
    }
    void Lerp4(Vec2<type> goal,type dist)
    {
        this->x += (type)(dist/this->Dist(goal) * (goal.x - this->x));
        this->y += (type)(dist/this->Dist(goal) * (goal.y - this->y));
    }
    float Dist(Vec2 point)   //D
    {
        return sqrt(
                   (point.x - this->x)*(point.x - this->x)
                   +
                   (point.y - this->y)*(point.y - this->y)

               );
        // return (type)sqrt(pow(point.x-this->x,2)+pow(point.y-this->y,2));
    };
    float GetAngle(Vec2<type> other)
    {
        return atan2(other.x - this->x,other.y - this->y);
    };
    type Length()
    {
        return((type)sqrt(x*x + y*y));
    };
    float FLength()
    {
        return(sqrt(x*x + y*y));
    };
    Vec2<type> Normalize()
    {
        if(this->Length() == 0)
            return Vec2<type>(0,0)
                   else
                       return Vec2<type>(this->x/this->FLength(),this->y/this->FLength());
    };
    void print()
    {
        printf("(%d,%d)",this->x,this->y);
    }
    type DotProduct(Vec2<type> v)
    {
        return (type)(x*v.x+y*v.y);

    }
    type CrossProduct(Vec2<type> v)
    {

        return (type)(x * v.y - y * v.x);
    }
};

template <typename type> class Vec3
{
public:
    type x,y,z;
    Vec3()
    {
        x = (type)0;
        y = (type)0;
        z = (type)0;
    };
    Vec3(type x1,type y1,type z1)
    {
        x = x1;
        y = y1;
        z = z1;
    };

    bool operator == (Vec3<type>comp)
    {
        return (x == comp.x) && (y == comp.y) && (z == comp.z);
    };
    type Dist(Vec3<type> Pos)
    {
        return (type)sqrt(pow(this->x-Pos.x,2)+pow(this->y-Pos.y,2)+pow(this->z-Pos.z,2));
    };
    type length()
    {
        return((type)sqrt(x*x + y*y + z*z));
    };
    Vec3<type> operator-(void)
    {
        return Vec3<type>(-x,-y,-z);
    };
    Vec3<type> operator-(Vec3<type>v)
    {
        return Vec3<type>(x-v.x,y-v.y,z-v.z);
    };
    Vec3<type> operator+(Vec3<type>v)
    {
        return Vec3<type>(x+v.x,y+v.y,z+v.z);
    };
    Vec3<type> Cross(Vec3<type> &v)
    {
        return Vec3<type>(y*v.z - z*v.y,z * v.x - x * v.z,x * v.y - y * v.x);
    };
    Vec3<type> operator*(type t)
    {
        return Vec3<type>(x*t,y*t,z*t);
    };
    Vec3<type> operator*(Vec3<type>v)
    {
        return this->Cross(v);//Vec3<type>(x*v.x,y*v.y,z*v.z);
    };
    Vec3<type> operator%(type mod)
    {
        return Vec3<float>(x%mod,y%mod,z%mod);
    };
    Vec3<type> operator%=(type mod)
    {
        this->x%=mod;
        this->y%=mod;
        this->z%=mod;
        return this;
    };
    Vec3<type> unit()
    {
        return Vec3<type>(x/length(),y/length(),z/length());
    };
    void Normalize()
    {
        type len = length();
        if (len)
        {
            x /= len;
            y /= len;
            z /= len;
        }
    };
    void normalize()
    {
        this->Normalize();
    };
    Vec3<type> Lerp(Vec3<type> goal,float alpha)
    {
        return Vec3<type>((type)(this->x + alpha * (goal.x - this->x)),(type)(this->y + alpha * (goal.y - this->y)),(type)(this->z + alpha * (goal.z - this->z)));
    }
    type innerProduct(Vec3<type> &v)
    {
        return (type)(x*v.x+y*v.y+z*v.z);
    };
    void copy(Vec3<type> v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
    };
    void copy(type x1,type y1,type z1)
    {
        x = x1;
        y = y1;
        z = z1;
    };
    void print()
    {
        printf("(%f,%f,%f)",this->x,this->y,this->z);
    };
    Vec2<type> GetAnglesR() //this will normalize the vector.
    {
        this->normalize();
        return Vec2<type>(atan(this->z/this->x),asin(this->y/sqrt(x*x+z*z)));

    };
    Vec2<type> GetAnglesD()
    {
        return (this->GetAnglesR())*(180/(3.14159));
    };
#ifdef m_vec_debug
    void Print()
    {
        printf("(%.1f,%.1f,%.1f)",this->x,this->y,this->z);

    };
#endif
};

};
#endif
