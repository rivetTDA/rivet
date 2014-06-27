Num_Points=20;
radius=1.5;
std_dev=.85;

a=[2*pi*rand(Num_Points,1),std_dev*randn(Num_Points,1)+3];
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
