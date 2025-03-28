#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

const int Lx=400;
const int Ly=100;

const int Q=9;

const double tau=0.8000;
const double Utau=1.0/tau;
const double UmUtau=1-Utau;



//--------------------- class LatticeBoltzmann ------------
class LatticeBoltzmann{
  private:
    double w[Q];      //Weights
    int Vx[Q],Vy[Q];  //Velocity vectors
    double *f, *fnew; //Distribution Functions
  public:
    LatticeBoltzmann(void);
    ~LatticeBoltzmann(void);
    int n(int ix,int iy,int i){return (ix*Ly+iy)*Q+i;};
    double rho(int ix,int iy,bool UseNew);
    double Jx(int ix,int iy,bool UseNew);
    double Jy(int ix,int iy,bool UseNew);
    double feq(double rho0,double Ux0,double Uy0,int i);
    void Collision(void);
    void ImposeFields(double Ufan, double a, double b, double d, double e);
    void Advection(void);
    vector<double> Derivatives(int ix, int iy, double dt); //Calcula dUx/dx, dUx/dy, dUy/dx, dUy/dy dada una celda.

    //Ya con el valor de las derivadas, se calculan las componentes del tensor de esfuerzos.
    double sigmaxx(double rho0, double eta, double dx_dx);
    double sigmaxy(double rho0, double eta, double dx_dy, double dy_dx);
    double sigmayy(double rho0, double eta, double dy_dy);

    //Función de interpolación bilineal
    vector<double> interpolationSigma(double x, double y, double nu, double dt);

    //Función que calcula el diferencial de fuerza
    vector<double> dF(double x, double y, double dx, double dy, double nu, double dt);
    vector<double> FOnCoin(double nu, double dt, int N, double a, double b, double d,double e);
  
    void Start(double rho0,double Ux0,double Uy0);
    void Print(const char * NameFile,double Ufan);
};  
LatticeBoltzmann::LatticeBoltzmann(void){
  //Set the weights
  w[0]=4.0/9; w[1]=w[2]=w[3]=w[4]=1.0/9; w[5]=w[6]=w[7]=w[8]=1.0/36;
  //Set the velocity vectors
  Vx[0]=0;  Vx[1]=1;  Vx[2]=0;  Vx[3]=-1; Vx[4]=0;
  Vy[0]=0;  Vy[1]=0;  Vy[2]=1;  Vy[3]=0;  Vy[4]=-1;

  Vx[5]=1;  Vx[6]=-1; Vx[7]=-1; Vx[8]=1;
  Vy[5]=1;  Vy[6]=1;  Vy[7]=-1; Vy[8]=-1;
  //Create the dynamic arrays
  int ArraySize=Lx*Ly*Q;
  f=new double [ArraySize];  fnew=new double [ArraySize];
}
LatticeBoltzmann::~LatticeBoltzmann(void){
  delete[] f;  delete[] fnew;
}
double LatticeBoltzmann::rho(int ix,int iy,bool UseNew){
  double sum; int i,n0;
  for(sum=0,i=0;i<Q;i++){
    n0=n(ix,iy,i);
    if(UseNew) sum+=fnew[n0]; else sum+=f[n0];
  }
  return sum;
}  
double LatticeBoltzmann::Jx(int ix,int iy,bool UseNew){
  double sum; int i,n0;
  for(sum=0,i=0;i<Q;i++){
    n0=n(ix,iy,i);
    if(UseNew) sum+=Vx[i]*fnew[n0]; else sum+=Vx[i]*f[n0];
  }
  return sum;
}  
double LatticeBoltzmann::Jy(int ix,int iy,bool UseNew){
  double sum; int i,n0;
  for(sum=0,i=0;i<Q;i++){
    n0=n(ix,iy,i);
    if(UseNew) sum+=Vy[i]*fnew[n0]; else sum+=Vy[i]*f[n0];
  }
  return sum;
}  
double LatticeBoltzmann::feq(double rho0,double Ux0,double Uy0,int i){
  double UdotVi=Ux0*Vx[i]+Uy0*Vy[i], U2=Ux0*Ux0+Uy0*Uy0;
  return rho0*w[i]*(1+3*UdotVi+4.5*UdotVi*UdotVi-1.5*U2);
}
void LatticeBoltzmann::Start(double rho0,double Ux0,double Uy0){
  int ix,iy,i,n0;
  for(ix=0;ix<Lx;ix++) //for each cell
    for(iy=0;iy<Ly;iy++)
      for(i=0;i<Q;i++){ //on each direction
        n0=n(ix,iy,i);
        f[n0]=feq(rho0,Ux0,Uy0,i);
      }
}  
void LatticeBoltzmann::Collision(void){
  int ix,iy,i,n0; double rho0,Ux0,Uy0;
  for(ix=0;ix<Lx;ix++) //for each cell
    for(iy=0;iy<Ly;iy++){
      //compute the macroscopic fields on the cell
      rho0=rho(ix,iy,false); Ux0=Jx(ix,iy,false)/rho0; Uy0=Jy(ix,iy,false)/rho0;
      for(i=0;i<Q;i++){ //for each velocity vector
        n0=n(ix,iy,i);
        fnew[n0]=UmUtau*f[n0]+Utau*feq(rho0,Ux0,Uy0,i);
      }
    }  
}

void LatticeBoltzmann::ImposeFields(double Ufan, double a, double b, double d, double e){
  int i,ix,iy,n0; double rho0,y1,y2,x1,x2;
  //go through all cells, looking if they are fan or obstacle

  y1= e; //Parte inferior
  y2= y1+b; //Parte superior
  x2= d;         //Parte derecha
  x1= x2-a; //Parte izquierda


  for(ix=0;ix<Lx;ix++){ //for each cell
    //Ecuaciones que describen la moneda

    for(iy=0;iy<Ly;iy++){
      rho0=rho(ix,iy,false);
      //fan
      if(ix==0 && (iy>y1 || iy==y1)){
        for(i=0;i<Q;i++){n0=n(ix,iy,i); fnew[n0]=feq(rho0,Ufan,0,i);}}

      //Obstáculo
      else if(iy<y2 && iy>y1 && ix>x1 && ix<x2){
        for(i=0;i<Q;i++) {n0=n(ix,iy,i); fnew[n0]=feq(rho0,0,0,i);}
      }
      else if(iy<(y1-10))
        for(i=0;i<Q;i++){n0=n(ix,iy,i); fnew[n0]=feq(rho0,0,0,i);}

      //An extra point at one side to break the isotropy
      // else if(ix==ixc && iy==iyc+R+1)
      //for(i=0;i<Q;i++){n0=n(ix,iy,i); fnew[n0]=feq(rho0,0,0,i);}	
    }
  }
}
void LatticeBoltzmann::Advection(void){
  int ix,iy,i,ixnext,iynext,n0,n0next;
  for(ix=0;ix<Lx;ix++) //for each cell
    for(iy=0;iy<Ly;iy++)
      for(i=0;i<Q;i++){ //on each direction
        ixnext=(ix+Vx[i]+Lx)%Lx; iynext=(iy+Vy[i]+Ly)%Ly;
        n0=n(ix,iy,i); n0next=n(ixnext,iynext,i);
        f[n0next]=fnew[n0]; //periodic boundaries
      }
}
vector<double> LatticeBoltzmann::Derivatives(int ix, int iy,double dt){
  int i,ixnext,iynext;
  double rhoNext,UxNext,UyNext;
  double sumxx=0, sumxy=0, sumyx=0, sumyy=0;

  for(i=0;i<Q;i++){ //on each direction
    ixnext=(ix+Vx[i]+Lx)%Lx; iynext=(iy+Vy[i]+Ly)%Ly;

    //Calculo las velocidades macroscópicas para las celdas aledañas a la celda en la que estoy parada

    rhoNext=rho(ixnext,iynext,false); UxNext=Jx(ixnext,iynext,false)/rhoNext; UyNext=Jy(ixnext,iynext,false)/rhoNext; //preguntarse si usar false o true: false si se usa luego de advección.

    //Posibles combinaciones de la sumatoria para calcular más adelante las derivadas (se muestra en el taller)

    sumxx += w[i]*Vx[i]*UxNext;
    sumxy += w[i]*Vy[i]*UxNext;
    sumyx += w[i]*Vx[i]*UyNext;
    sumyy += w[i]*Vy[i]*UyNext;
	
  }

  //Ya las posibles derivadas calculadas

	double dx_dx = (3.0/dt)*sumxx;
  double dx_dy = (3.0/dt)*sumxy; //dx_dy es distinto a dy_dx 
	double dy_dx = (3.0/dt)*sumyx;
	double dy_dy = (3.0/dt)*sumyy;

  //Vector que guarda las derivadas

  vector<double> derivs = {dx_dx, dx_dy, dy_dx, dy_dy};
  return derivs;
}

//Calcula la componente xx del tensor de esfuerzos
double LatticeBoltzmann::sigmaxx(double rho0, double eta, double dx_dx){
  double p = rho0/3.0;
  double sxx = -p+eta*(2*dx_dx);
  return sxx;
}

//Calcula la componente xy e yx (son iguales) del tensor de esfuerzos
double LatticeBoltzmann::sigmaxy(double rho0, double eta, double dx_dy, double dy_dx){
  double sxy = eta*(dx_dy + dy_dx);
  return sxy;
}
//Calcula la componente yy del tensor de esfuerzos
double LatticeBoltzmann::sigmayy(double rho0, double eta, double dy_dy){
  double p = rho0/3.0;
  double syy = -p+eta*(2*dy_dy);
  return syy;
}

vector<double> LatticeBoltzmann::interpolationSigma(double x, double y, double nu, double dt){
/*
  M es el valor de la celda a la que pertenece un (x,y) arbitrario.
  M*ancho = x + 0  ;   x: punto cualquiera que mido en el plano
  M = x/ancho

  Lx=5
  0<=ix<=4

  |__0__|__1__|__2__|__3__|__4__|

  x=2.5, ¿A qué ix corresponde?
  M=ix= int(2.5/ancho) = 2
*/

  int ix = static_cast<int>(x);
  int iy = static_cast<int>(y);
  double u = x - ix;
  double v = y - iy;

//Cálculo para celda (ix,iy)
  double rho_ix_iy = rho(ix,iy,false);
  double eta_ix_iy = nu*rho_ix_iy;

  vector<double> derivatesIxIy = Derivatives(ix, iy, dt); //Devuelve las derivadas {dUx_dx, dUx_dy, dUy_dx, dUy_dy}
  double sigmaxx_ix_iy = sigmaxx(rho_ix_iy, eta_ix_iy, derivatesIxIy[0]);
  double sigmaxy_ix_iy = sigmaxy(rho_ix_iy, eta_ix_iy, derivatesIxIy[1],derivatesIxIy[2]);
  double sigmayy_ix_iy = sigmayy(rho_ix_iy, eta_ix_iy, derivatesIxIy[3]);

//Cálculo para (ix+1,iy)
  double rho_ixP1_iy = rho(ix+1,iy,false);
  double eta_ixP1_iy = nu*rho_ixP1_iy;

  vector<double> derivatesIxP1Iy = Derivatives(ix+1, iy, dt); //Devuelve las derivadas {dUx_dx, dUx_dy, dUy_dx, dUy_dy}
  double sigmaxx_ixP1_iy = sigmaxx(rho_ixP1_iy, eta_ixP1_iy, derivatesIxP1Iy[0]);
  double sigmaxy_ixP1_iy = sigmaxy(rho_ixP1_iy, eta_ixP1_iy, derivatesIxP1Iy[1],derivatesIxP1Iy[2]);
  double sigmayy_ixP1_iy = sigmayy(rho_ixP1_iy, eta_ixP1_iy, derivatesIxP1Iy[3]);

//Cálculo para (ix,iy+1) [Problema si iy es el límite.]
  double rho_ix_iyP1 = rho(ix,iy+1,false);
  double eta_ix_iyP1 = nu*rho_ix_iyP1;

  vector<double> derivatesIxIyP1 = Derivatives(ix, iy+1, dt); //Devuelve las derivadas {dUx_dx, dUx_dy, dUy_dx, dUy_dy}
  double sigmaxx_ix_iyP1 = sigmaxx(rho_ix_iyP1, eta_ix_iyP1, derivatesIxIyP1[0]);
  double sigmaxy_ix_iyP1 = sigmaxy(rho_ix_iyP1, eta_ix_iyP1, derivatesIxIyP1[1],derivatesIxIyP1[2]);
  double sigmayy_ix_iyP1 = sigmayy(rho_ix_iyP1, eta_ix_iyP1, derivatesIxIyP1[3]);


//Cálculo para (ix+1,iy+1) [Problema si iy es el límite.]
  double rho_ixP1_iyP1 = rho(ix+1,iy+1,false);
  double eta_ixP1_iyP1 = nu*rho_ixP1_iyP1;

  vector<double> derivatesIxP1IyP1 = Derivatives(ix+1, iy+1, dt); //Devuelve las derivadas {dUx_dx, dUx_dy, dUy_dx, dUy_dy}
  double sigmaxx_ixP1_iyP1 = sigmaxx(rho_ixP1_iyP1, eta_ixP1_iyP1, derivatesIxP1IyP1[0]);
  double sigmaxy_ixP1_iyP1 = sigmaxy(rho_ixP1_iyP1, eta_ixP1_iyP1, derivatesIxP1IyP1[1],derivatesIxP1IyP1[2]);
  double sigmayy_ixP1_iyP1 = sigmayy(rho_ixP1_iyP1, eta_ixP1_iyP1, derivatesIxP1IyP1[3]);

  //Se calcula el sigma interpolado de acuerdo a la fórmula dada.
  double interpolatedSigmaXX = sigmaxx_ix_iy*(1-u)*(1-v) + sigmaxx_ixP1_iy*u*(1-v) + sigmaxx_ix_iyP1*(1-u)*v + sigmaxx_ixP1_iyP1*u*v;
  double interpolatedSigmaXY = sigmaxy_ix_iy*(1-u)*(1-v) + sigmaxy_ixP1_iy*u*(1-v) + sigmaxy_ix_iyP1*(1-u)*v + sigmaxy_ixP1_iyP1*u*v;
  double interpolatedSigmaYY = sigmayy_ix_iy*(1-u)*(1-v) + sigmayy_ixP1_iy*u*(1-v) + sigmayy_ix_iyP1*(1-u)*v + sigmayy_ixP1_iyP1*u*v;

  vector <double> interpolatedSigma = {interpolatedSigmaXX, interpolatedSigmaXY, interpolatedSigmaYY};
  return interpolatedSigma;

 
}
vector<double> LatticeBoltzmann::dF(double x,double y, double dx, double dy, double nu, double dt){


  vector<double> sigmaInterpolated = interpolationSigma(x,y,nu,dt); //Se obtiene sigma interpolado.

  /*
  Se calcula el diferencial de fuerza como:

  dFi = sigma(ij) dAj
  * dFx = sigma(xx)dAx + sigma(xy)dAy
  * dFy = sigma(yx)dAx + sigma(yy)dAy

  dA = (dx,dy)
*/
  vector<double> fuerza = {sigmaInterpolated[0]*dx+sigmaInterpolated[1]*dy,sigmaInterpolated[1]*dx+sigmaInterpolated[2]*dy};

  return fuerza;



}

vector<double> LatticeBoltzmann::FOnCoin(double nu, double dt, int N, double a, double b, double d, double e){

/*
  - Recibe el número N de elementos en los que se divide cada fragmento.
  - Todos los subíndices 1 corresponden a la recta creciente, los subíndices 2 a la recta decreciente y los subíndices 3 a la vertical.
  Se tiene que el triángulo está compuesto por 3 puntos: A=(d-a, Ly/2) , B=(d, Ly/2+b/2) y C= (d, Ly/2-b/2)
  La ecuación de y1 es y1(x)= ((b/2)*a)*(x-d+a) + (Ly/2), de modo que si quiero partir el segmento en 16 partes tendría
  que cada punto correspondiente al lugar donde se realiza una partición sería:
  puntoPartición1 = A + (m/N)(a,b/2) donde observe que (a, b/2) viene de la información de la pendiente, pues esa recta
  siempre avanza b/2 pasos en y por cada a pasos en x.
  Si estamos interesados en medir los puntos centrales de cada segmento que dividimos, basta con sumarle la puntoPartición1
  la mitad del primer segmento, para que salte de punto central en punto central.
  Así:
  puntoCentral1 = puntoPartición1 + (1/(2N))*(a,b/2) = A + (1/(2N))*(a,b/2) + (m/N)(a,b/2)
  Así, puntoCentral1 = (d-a, Ly/2) + (1/(2N))*(a,b/2) + (m/N)(a,b/2) , que es lo implementado en el código. (m va desde 0 hasta N-1)

  Con un procedimiento similar, se encontró que:
  puntoCentral2 = (d, Ly/2+b/2) + (1/(2N))*(a,-b/2) + (m/N)(a,-b/2)
  puntoCentral3 = (d, Ly/2-b/2) + (1/(2N))*(B-C) + (m/N)(B-C)

  Ahora, para los vectores normales, basta calcular uno por cada segmento del triángulo, pues es el mismo vector normal
  para todas las particiones del segmento.
  Para el de la línea creciente, tenemos que el vector que va desde A hasta su primera partición es:
  puntoPartición1(m=1) - A = (1/N)(a,b/2). Este es el vector que une a A y a su primer punto de partición, por
  lo que su longitud será dl. Ahora falta la dirección.
  Si tomamos un vector (x,y), tenemos que (-y,x) siempre va a ser perpendicular a él, por lo que en este caso el vector
  normal corresponde a:
  vectorNormal1 = (1/N)(-b/2,a)

  Para el caso de la recta decreciente, se sigue un procedimiento similar, pero en vez de usar (-y,x) se usa (y,-x) para que
  el vector de área apunte hacia afuera del triángulo.
  Así,

  vectorNormal2 = (1/N)(-b/2,-a)

  Por último, para la recta vertical, basta con calcular la norma de una partición y hacerla ir en dirección x.
  vectorNormal3 = (0,b/N)

*/

  double aD2n = (a/2.0)*(1.0/N);
  double aDn = (a)*(1.0/N);
  double bD2n = (b/2.0)*(1.0/N);
  double bDn = (b)*(1.0/N);

  double vectorNormaly_x = 0.0; //Componente x del vector normal de la recta y=y1 ó y=y2
  double vectorNormaly_y = aDn; //Componente y del vector normal de la recta y=y1 ó y=y2
  double vectorNormalx_x = bDn;
  double vectorNormalx_y = 0.0;



  // cout<<"VectorNormal1: ("<<vectorNormal1[0]<<" "<<vectorNormal1[1]<<")"<<"\n";

  //Guardan la fuerza total.
  double fTotalX = 0;
  double fTotalY = 0;


  vector<double> vectorInicialy1 = {(d-a) + aD2n , e};
  vector<double> vectorInicialy2 = {(d-a) + aD2n, e+b};
  vector<double> vectorInicialx1 = {d-a,e+bD2n};
  vector<double> vectorInicialx2 = {d,bD2n+e};

  double y1Central_y = vectorInicialy1[1];
  double y2Central_y = vectorInicialy2[1];
  double x1Central_x = vectorInicialx1[0];
  double x2Central_x = vectorInicialx2[0];

  double y1Central_x, y2Central_x, x1Central_y,x2Central_y;

  //Se recorre cada línea.
  for (int m=0;m<N; m++) {

    y1Central_x = vectorInicialy1[0] + m*aDn;
    y2Central_x = vectorInicialy2[0] + m*aDn;
    x1Central_y = vectorInicialx1[1] + m*bDn;
    x2Central_y = vectorInicialx2[1] + m*bDn;

    // cout<<"\n"<<"Vectorx1y1Central: ("<<x1Central<<" "<<y1Central<<")"<<" m:"<<m<<"\n";
    // dF(x1Central,y1Central,dx1,dy1,nu,dt)
    vector<double> fIteracionY1 = dF(y1Central_x,y1Central_y,vectorNormaly_x,(-1.0)*vectorNormaly_y,nu,dt); //Se calcula el dF debido al segmento de la línea inferior.
    vector<double> fIteracionY2 = dF(y2Central_x,y2Central_y,vectorNormaly_x,vectorNormaly_y, nu, dt); //Se calcula el dF debido al segmento de la línea superior.
    vector<double> fIteracionX1 = dF(x1Central_x,x1Central_y,(-1.0)*vectorNormalx_x,vectorNormalx_y,nu,dt); //Se calcula el dF debido al segmento de la línea izquierda.
    vector<double> fIteracionX2 = dF(x2Central_x,x2Central_y,vectorNormalx_x,vectorNormalx_y,nu,dt); //Se calcula el dF debido al segmento de la línea derecha.


    fTotalX += fIteracionY1[0]+fIteracionY2[0]+fIteracionX1[0]+fIteracionX2[0];
    fTotalY += fIteracionY1[1]+fIteracionY2[1]+fIteracionX1[1]+fIteracionX2[1];
  }
  vector<double> fuerzaSobreMoneda = {fTotalX,fTotalY};
  return fuerzaSobreMoneda;
}




void LatticeBoltzmann::Print(const char * NameFile,double Ufan){
  ofstream MyFile(NameFile); int ix,iy; double rho0,Ux0,Uy0;
  for(ix=0;ix<Lx;ix+=5){
    for(iy=0;iy<Ly;iy+=5){
      rho0 = rho(ix,iy,true); Ux0=Jx(ix,iy,true)/rho0; Uy0=Jy(ix,iy,true)/rho0;
      MyFile<<ix<<" "<<iy<<" "<<(3*Ux0)/Ufan<<" "<<(3*Uy0)/Ufan<<endl;
    }
    MyFile<<endl;
  }
  MyFile.close();
}

//--------------- Global Functions ------------




int main(int argc, char *argv[]) {

  LatticeBoltzmann Air;
  int t,tmax=3000;
  double rho0=1.0;
  double Ufan0 = 0.139;
  double dt = 1.0;
  double a = 90; //Ancho
  double b = 10; //Largo
  double d = Lx/4+a; //Ubicación del lado derecho en eje x
  double e = 50; //Ubicación del lado inferior en eje y
  double nu = (1/3.0)*(tau- 1.0/2);
  int N = 36;
  vector<double> fCoin = {0,0};

  double Fx;
  double Fy;


  ofstream fout;
  fout.open("FxFy.dat");

  //Start
  Air.Start(rho0,Ufan0,0);
  //Run
  for(t=0;t<tmax;t++){
    Air.Collision();
    Air.ImposeFields(Ufan0,a,b,d,e);
    Air.Advection();
    fCoin = Air.FOnCoin(nu, dt, N, a,b, d, e); //Se calcula la fuerza total sobre la moneda.
    Fx = fCoin[0];
    Fy = fCoin[1];
    fout<<t<<" "<<Fx<<" "<<Fy<<"\n";

  }

 fout.close();

 //Show
 Air.Print("wind.dat",Ufan0);

  return 0;
}  


///https://github.com/jviquerat/lbm/tree/master/lbm/src/app
