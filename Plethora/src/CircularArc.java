import java.util.ArrayList;

public class CircularArc {

	private Long id;
	private Vertices[] vert = new Vertices[2];
	private double centerX;
	private double centerY;
	private double radius;
	private Vertices startVert; //clockwisefrom
	
	public CircularArc(Long edgeID, Vertices[] vert, double centerX, double centerY)
	{
		setId(edgeID);
		setVert(vert);
		this.setCenterX(centerX);
		this.setCenterY(centerY);
	}
//
	public Long getId() {
		return id;
	}

	public void setId(Long id) {
		this.id = id;
	}	

	public double getCenterX() {
		return centerX;
	}

	public void setCenterX(double centerX) {
		this.centerX = centerX;
	}

	public double getCenterY() {
		return centerY;
	}

	public void setCenterY(double centerY) {
		this.centerY = centerY;
	}

	public Vertices getStartVert() {
		return startVert;
	}

	public void setStartVert(Vertices startVert) {
		this.startVert = startVert;
	}

	public double getRadius()
	{
		radius = Math.sqrt(((centerX - startVert.getX())*(centerX - startVert.getX())) 
				+ ((centerY - startVert.getY())*(centerY - startVert.getY())));
		return radius;
	}

	public Vertices[] getVert() {
		return vert;
	}

	public void setVert(Vertices[] vert) {
		this.vert = vert;
	}
	
}
