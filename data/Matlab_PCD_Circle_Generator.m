Num_Signal_Points=200;
Num_Noise_Points=40;
radius=1.5;
std_dev_signal=.3;
std_dev_noise=1.3;

Num_Points=Num_Signal_Points+Num_Noise_Points;

a=[2*pi*rand(Num_Points,1),randn(Num_Points,1)];
a(1:Num_Signal_Points,2)=std_dev_signal*a(1:Num_Signal_Points,2)+3;
a(Num_Signal_Points+1:Num_Points,2)=std_dev_noise*a(Num_Signal_Points+1:Num_Points,2)+3;

points=zeros(Num_Points,2);
for i=1:Num_Points
    points(i,:)=a(i,2)*[cos(a(i,1)),sin(a(i,1))];
end
points

Density_Values=zeros(Num_Points,1)
for i=1:Num_Points
    for j=1:Num_Points
        if norm(points(i,:)-points(j,:))<=radius
        Density_Values(i)=Density_Values(i)+1;
        end
    end
end
Density_Values
