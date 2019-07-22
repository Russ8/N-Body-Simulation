



__kernel void main_( __global float* pos, __global float* vel, __global const float* mass)	
{
		
		const int nParticles = 40000;
		const float acc_factor = 100.0f;
		const float vel_damper = 1.0f;
		const float mass_damper = 0.1f; 

		int id = get_global_id(0); 
		float3 id_pos = (float3)(pos[id * 3], pos[id * 3 + 1], pos[id * 3 + 2]);
		float id_mass = mass[id];
		int i = 0;
		
		float acc_x = 0;
		float acc_y = 0;
		float acc_z = 0;

		for (i; i<nParticles; i++) {
		
			float3 i_pos = (float3)(pos[i * 3], pos[i * 3 + 1], pos[i * 3 + 2]);
			
			float distance = sqrt(((i_pos.x - id_pos.x) * (i_pos.x - id_pos.x)) + ((i_pos.y - id_pos.y) * (i_pos.y - id_pos.y)) + ((i_pos.z - id_pos.z) * (i_pos.z - id_pos.z)));
			float d2 = distance * distance;
			float3 dir = i_pos - id_pos;
			dir = normalize(dir);

			if(d2 ==0) { 
				d2 = 0.00001;
			}

			float mass_product = (id_mass * mass[i] * mass_damper);

			acc_x = acc_x + ((mass_product * dir.x) / d2);
			acc_y = acc_y + ((mass_product * dir.y) / d2);
			acc_z = acc_z + ((mass_product * dir.z) / d2);

		}

		float vx = vel[id * 3] + (acc_factor * acc_x);
		float vy = vel[id * 3 + 1] + (acc_factor * acc_y);
		float vz = vel[id * 3 + 2] + (acc_factor * acc_z);

		vel[id * 3] = vx ;
		vel[id * 3 + 1] = vy ;
		vel[id * 3 + 2] = vz ;

		pos[id * 3] = id_pos.x + (vel_damper * vx);
		pos[id * 3 + 1] = id_pos.y + (vel_damper * vy);
		pos[id * 3 + 2] = id_pos.z + (vel_damper * vz);

}
	
	




