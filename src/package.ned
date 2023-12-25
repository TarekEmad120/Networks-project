package networks_project;

@license(LGPL);
//
// TODO documentation
//
network Network
{
    parameters:
        int    WS= default(3);
        int    WR= default(3);
        int    TO= default(10);
        double PT= default(0.5);
        double TD= default(1.0);
        double ED= default(4.0);
        double DD= default(0.1);
        double LP= 0;
        
    @display("bgb=602,211");
    submodules:
        node0: Node {
            @display("p=173,73");
            node_id = 0;
        }
        node1: Node {
            @display("p=449,78");
            node_id = 1;
        }
        coordinator: Coordinator {
            @display("p=314,160");
        }
    connections:
        node0.Nodeout --> node1.Nodein;
        node1.Nodeout --> node0.Nodein;
        coordinator.Coordinate_node0 --> node0.Coordinator;
        coordinator.Coordinate_node1 --> node1.Coordinator;
}

